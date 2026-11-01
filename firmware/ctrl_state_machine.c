/*
 * 板端安全状态机
 * ================
 * 把 PC 端的摇杆状态机下沉到 MC632X，使 PC 异常时板端仍能安全收敛到停车。
 *
 * 判定链路：
 *   ① 收到 PC 数据？        超过 1 秒没收到 -> 超时停车
 *   ② PC 报急停？            是 -> 急停锁存，直到手离开才解除
 *   ③ 幅度够吗？             < MIN_SPEED -> 停止
 *   ④ 方向有效吗？           映射为指令
 *
 * 四态：STANDBY / READY / ACTIVE / EMERGENCY
 *
 * 输出：串口 [CMD] <w|s|a|d|wl|wr|sl|sr|stop>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>

/* ─────────── 外部接口（cmd_output_server.c）─────────── */
extern void cmdout_set(const char *cmd);
extern int  cmdout_start(void);
extern void cmdout_stop(void);

/* ─────────── 配置 ─────────── */
#define CTRL_PORT        9001
#define RECV_BUF         256
#define MIN_SPEED        60      /* 幅度门限，0~1000 */
#define TIMEOUT_MS       1000    /* 超过此时间没收到 PC 数据即停车 */

/* ─────────── 状态定义 ─────────── */
enum {
    ST_STANDBY = 0,
    ST_READY,
    ST_ACTIVE,
    ST_EMERGENCY
};

static const char *state_name(int s)
{
    switch (s) {
    case ST_STANDBY:   return "STANDBY";
    case ST_READY:     return "READY";
    case ST_ACTIVE:    return "ACTIVE";
    case ST_EMERGENCY: return "EMERGENCY";
    default:           return "?";
    }
}

/* ─────────── 内部状态 ─────────── */
typedef struct {
    int      state;
    int      estop_latched;   /* 急停锁存 */
    char     last_cmd[8];     /* 上次输出的指令，用于变化检测 */
    int      last_dir_x;      /* 方向分量，用于判断是否回中 */
    int      last_dir_y;
    int      last_mag;
    uint32_t last_recv_ms;    /* 上次收到数据的时刻 */
    int      has_recv;        /* 是否收到过数据 */
    unsigned long rx_count;
    unsigned long tx_count;
} ctrl_ctx_t;

static ctrl_ctx_t     g_ctx;
static pthread_mutex_t g_ctx_lock = PTHREAD_MUTEX_INITIALIZER;
static volatile int   g_running = 0;
static int            g_listen_fd = -1;

/* ─────────── 工具 ─────────── */
static uint32_t now_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint32_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

/* 方向字符串 -> 指令 */
static const char *dir_to_cmd(const char *dir)
{
    if (!dir) return "stop";
    if (!strcmp(dir, "FWD"))        return "w";
    if (!strcmp(dir, "BACK"))       return "s";
    if (!strcmp(dir, "LEFT"))       return "a";
    if (!strcmp(dir, "RIGHT"))      return "d";
    if (!strcmp(dir, "FWD_LEFT"))   return "wl";
    if (!strcmp(dir, "FWD_RIGHT"))  return "wr";
    if (!strcmp(dir, "BACK_LEFT"))  return "sl";
    if (!strcmp(dir, "BACK_RIGHT")) return "sr";
    return "stop";
}

/* 方向字符串 -> 单位向量（用于判断是否回中） */
static void dir_to_vec(const char *dir, int *x, int *y)
{
    *x = 0; *y = 0;
    if (!dir) return;
    if (!strcmp(dir, "FWD"))        { *y =  1; return; }
    if (!strcmp(dir, "BACK"))       { *y = -1; return; }
    if (!strcmp(dir, "LEFT"))       { *x = -1; return; }
    if (!strcmp(dir, "RIGHT"))      { *x =  1; return; }
    if (!strcmp(dir, "FWD_LEFT"))   { *x = -1; *y =  1; return; }
    if (!strcmp(dir, "FWD_RIGHT"))  { *x =  1; *y =  1; return; }
    if (!strcmp(dir, "BACK_LEFT"))  { *x = -1; *y = -1; return; }
    if (!strcmp(dir, "BACK_RIGHT")) { *x =  1; *y = -1; return; }
}

/*
 * 状态机核心：根据 PC 发来的数据更新状态，返回应输出的指令
 *
 * dir : 方向字符串（FWD / FWD_LEFT / STOP ...）
 * mag : 幅度 0~1000
 * estop : PC 侧判定的急停
 * recv_ok : 本次调用是否收到了新数据
 */
static const char *sm_update(const char *dir, int mag, int estop, int recv_ok)
{
    ctrl_ctx_t *c = &g_ctx;
    uint32_t t = now_ms();
    const char *cmd = "stop";
    int vx, vy;

    if (recv_ok) {
        c->last_recv_ms = t;
        c->has_recv = 1;
        c->last_mag = mag;
        dir_to_vec(dir, &vx, &vy);
        c->last_dir_x = vx;
        c->last_dir_y = vy;
    }

    /* ① 超时看门狗：超过 TIMEOUT_MS 没收到数据 -> 强制停车 */
    if (!c->has_recv || (t - c->last_recv_ms) > TIMEOUT_MS) {
        if (c->state != ST_STANDBY) {
            printf("[SM] TIMEOUT (%ums) -> STANDBY\n", (unsigned)(t - c->last_recv_ms));
            c->state = ST_STANDBY;
        }
        c->estop_latched = 0;
        return "stop";
    }

    /* ② 急停：PC 报急停 -> 锁存 */
    if (estop) {
        if (!c->estop_latched) {
            printf("[SM] EMERGENCY (estop from PC)\n");
        }
        c->estop_latched = 1;
        c->state = ST_EMERGENCY;
        return "stop";
    }

    /* 急停解除条件：手回中（方向为 STOP）且收到数据 */
    if (c->estop_latched) {
        if (dir && (!strcmp(dir, "STOP") || mag < MIN_SPEED)) {
            printf("[SM] EMERGENCY released -> STANDBY\n");
            c->estop_latched = 0;
            c->state = ST_STANDBY;
        } else {
            c->state = ST_EMERGENCY;
            return "stop";
        }
    }

    /* ③ 无方向或幅度不足 -> 就绪（手在工作区但不驱动）*/
    if (!dir || !strcmp(dir, "STOP") || mag < MIN_SPEED) {
        if (c->state == ST_ACTIVE) {
            printf("[SM] %s -> READY (mag=%d)\n", state_name(c->state), mag);
        }
        c->state = ST_READY;
        return "stop";
    }

    /* ④ 正常驱动 */
    cmd = dir_to_cmd(dir);
    if (c->state != ST_ACTIVE) {
        printf("[SM] READY -> ACTIVE (%s mag=%d -> %s)\n", dir, mag, cmd);
        c->state = ST_ACTIVE;
    }
    return cmd;
}

/* 输出指令（只在变化时打印，避免刷屏） */
static void sm_emit(const char *cmd)
{
    ctrl_ctx_t *c = &g_ctx;

    if (strcmp(cmd, c->last_cmd) != 0) {
        strncpy(c->last_cmd, cmd, sizeof(c->last_cmd) - 1);
        c->last_cmd[sizeof(c->last_cmd) - 1] = 0;
        c->tx_count++;
        /* 串口日志 */
        printf("[CMD] %s\n", cmd);
        /* 网络输出：推给连在 9002 的执行端 */
        cmdout_set(cmd);
    }
}

/* ─────────── 解析 PC 数据 ───────────
 * 格式：dir=FWD_LEFT mag=520 estop=0
 */
static int parse_ctrl_line(const char *line, char *dir, int dir_sz,
                           int *mag, int *estop)
{
    const char *p;

    dir[0] = 0; *mag = 0; *estop = 0;

    p = strstr(line, "dir=");
    if (p) {
        p += 4;
        int i = 0;
        while (*p && *p != ' ' && *p != '\r' && *p != '\n' && i < dir_sz - 1) {
            dir[i++] = *p++;
        }
        dir[i] = 0;
    } else {
        return -1;
    }

    p = strstr(line, "mag=");
    if (p) *mag = atoi(p + 4);

    p = strstr(line, "estop=");
    if (p) *estop = atoi(p + 6);

    return 0;
}

/* ─────────── 控制线程 ─────────── */
static void *ctrl_thread(void *arg)
{
    struct sockaddr_in addr, cli;
    socklen_t cli_len;
    int conn_fd = -1, n, opt = 1;
    char buf[RECV_BUF + 1];
    char rx_buf[RECV_BUF * 2];
    int rx_len = 0;
    char dir[32];
    int mag, estop;

    (void)arg;

    g_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (g_listen_fd < 0) {
        printf("[SM] socket() failed\n");
        return NULL;
    }

    setsockopt(g_listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(CTRL_PORT);

    if (bind(g_listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("[SM] bind(%d) failed\n", CTRL_PORT);
        close(g_listen_fd);
        g_listen_fd = -1;
        return NULL;
    }

    if (listen(g_listen_fd, 4) < 0) {
        printf("[SM] listen() failed\n");
        close(g_listen_fd);
        g_listen_fd = -1;
        return NULL;
    }

    printf("[SM] ctrl server listening on %d (min_speed=%d timeout=%dms)\n",
           CTRL_PORT, MIN_SPEED, TIMEOUT_MS);

    g_running = 1;

    while (g_running) {
        fd_set rfds;
        struct timeval tv;
        int maxfd;

        /* 没有连接时只等 listen fd，1 秒超时用于跑看门狗 */
        FD_ZERO(&rfds);
        FD_SET(g_listen_fd, &rfds);
        maxfd = g_listen_fd;
        if (conn_fd >= 0) {
            FD_SET(conn_fd, &rfds);
            if (conn_fd > maxfd) maxfd = conn_fd;
        }

        tv.tv_sec = 1;
        tv.tv_usec = 0;

        if (select(maxfd + 1, &rfds, NULL, NULL, &tv) <= 0) {
            /* 超时：跑一次状态机检查看门狗 */
            pthread_mutex_lock(&g_ctx_lock);
            sm_emit(sm_update(NULL, g_ctx.last_mag, 0, 0));
            pthread_mutex_unlock(&g_ctx_lock);
            continue;
        }

        /* 新连接 */
        if (FD_ISSET(g_listen_fd, &rfds)) {
            cli_len = sizeof(cli);
            int nfd = accept(g_listen_fd, (struct sockaddr *)&cli, &cli_len);
            if (nfd >= 0) {
                if (conn_fd >= 0) {
                    printf("[SM] replace old client\n");
                    close(conn_fd);
                }
                conn_fd = nfd;
                rx_len = 0;
                printf("[SM] PC connected: %s:%d\n",
                       inet_ntoa(cli.sin_addr), ntohs(cli.sin_port));
            }
        }

        /* 收数据 */
        if (conn_fd >= 0 && FD_ISSET(conn_fd, &rfds)) {
            n = recv(conn_fd, buf, RECV_BUF, 0);
            if (n <= 0) {
                printf("[SM] PC disconnected -> force stop\n");
                close(conn_fd);
                conn_fd = -1;
                rx_len = 0;
                pthread_mutex_lock(&g_ctx_lock);
                g_ctx.has_recv = 0;
                sm_emit("stop");
                pthread_mutex_unlock(&g_ctx_lock);
            } else {
                buf[n] = 0;
                /* 处理粘包：按行切分 */
                if (rx_len + n < (int)sizeof(rx_buf)) {
                    memcpy(rx_buf + rx_len, buf, n);
                    rx_len += n;
                    rx_buf[rx_len] = 0;
                } else {
                    rx_len = 0;
                    rx_buf[0] = 0;
                }

                char *line = rx_buf;
                char *nl;
                while ((nl = strchr(line, '\n')) != NULL) {
                    *nl = 0;
                    if (parse_ctrl_line(line, dir, sizeof(dir), &mag, &estop) == 0) {
                        pthread_mutex_lock(&g_ctx_lock);
                        g_ctx.rx_count++;
                        sm_emit(sm_update(dir, mag, estop, 1));
                        pthread_mutex_unlock(&g_ctx_lock);
                    }
                    line = nl + 1;
                }
                /* 残留部分移到头部 */
                rx_len = strlen(line);
                memmove(rx_buf, line, rx_len + 1);

                /* 回 ACK */
                send(conn_fd, "ACK\n", 4, 0);
            }
        }
    }

    if (conn_fd >= 0) close(conn_fd);
    close(g_listen_fd);
    g_listen_fd = -1;
    printf("[SM] stopped\n");
    return NULL;
}

/* ─────────── 对外接口 ─────────── */
int ctrl_sm_start(void)
{
    pthread_t tid;

    /* 先启动 9002 输出服务，保证状态机首条指令能推出去 */
    cmdout_start();

    memset(&g_ctx, 0, sizeof(g_ctx));
    g_ctx.state = ST_STANDBY;
    strcpy(g_ctx.last_cmd, "stop");

    if (pthread_create(&tid, NULL, ctrl_thread, NULL) != 0) {
        printf("[SM] pthread_create failed\n");
        return -1;
    }
    pthread_detach(tid);
    return 0;
}

void ctrl_sm_stop(void)
{
    g_running = 0;
}
