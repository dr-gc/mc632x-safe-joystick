/*
 * 板端控制输出服务（端口 9002）
 * ==============================
 * 把状态机输出的控制指令通过 TCP 主动推给外部执行端。
 *
 * 架构：
 *   PC  --(TCP 9001)-->  板端状态机  -->  [CMD] w
 *                                          |
 *                                          v
 *                                    本模块（TCP 9002）
 *                                          |
 *                                          v
 *                                    执行端/仿真机（主动连板端）
 *
 * 协议：纯文本行，一行一条指令
 *   板端 -> 执行端：  w\n  /  sr\n  /  stop\n
 *   执行端 -> 板端：  可发任意心跳，板端忽略内容（只用于保活）
 *
 * 看门狗：执行端 3 秒没发心跳，板端不发新指令（由执行端自己判断停车）
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

#define OUT_PORT         9002
#define OUT_MAX_CLIENTS  4
#define OUT_HB_MS        500    /* 心跳周期：即使指令不变也重发，防执行端看门狗误触 */

typedef struct {
    int fd;
    int active;
    char addr[32];
} out_client_t;

static out_client_t   g_clients[OUT_MAX_CLIENTS];
static int            g_listen_fd = -1;
static volatile int   g_running = 0;
static pthread_mutex_t g_out_lock = PTHREAD_MUTEX_INITIALIZER;

/* 当前指令，由状态机通过 cmdout_set() 更新 */
static char           g_cur_cmd[8] = "stop";
static uint32_t       g_last_send_ms = 0;

static uint32_t out_now_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint32_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

/* ─────────── 对外接口：状态机调用 ─────────── */
void cmdout_set(const char *cmd)
{
    if (!cmd) return;
    pthread_mutex_lock(&g_out_lock);
    strncpy(g_cur_cmd, cmd, sizeof(g_cur_cmd) - 1);
    g_cur_cmd[sizeof(g_cur_cmd) - 1] = 0;
    pthread_mutex_unlock(&g_out_lock);
}

int cmdout_client_count(void)
{
    int i, n = 0;
    pthread_mutex_lock(&g_out_lock);
    for (i = 0; i < OUT_MAX_CLIENTS; i++)
        if (g_clients[i].active) n++;
    pthread_mutex_unlock(&g_out_lock);
    return n;
}

/* ─────────── 向所有客户端广播 ─────────── */
static void broadcast(const char *msg, int len)
{
    int i;
    pthread_mutex_lock(&g_out_lock);
    for (i = 0; i < OUT_MAX_CLIENTS; i++) {
        if (!g_clients[i].active) continue;
        if (send(g_clients[i].fd, msg, len, 0) < 0) {
            printf("[OUT] client %s gone\n", g_clients[i].addr);
            close(g_clients[i].fd);
            g_clients[i].active = 0;
            g_clients[i].fd = -1;
        }
    }
    pthread_mutex_unlock(&g_out_lock);
}

static void drop_client(int idx)
{
    pthread_mutex_lock(&g_out_lock);
    if (g_clients[idx].active) {
        printf("[OUT] client %s disconnected\n", g_clients[idx].addr);
        close(g_clients[idx].fd);
        g_clients[idx].active = 0;
        g_clients[idx].fd = -1;
    }
    pthread_mutex_unlock(&g_out_lock);
}

/* ─────────── 主线程 ─────────── */
static void *out_thread(void *arg)
{
    struct sockaddr_in addr, cli;
    socklen_t cli_len;
    int i, opt = 1;
    char buf[256];

    (void)arg;

    memset(g_clients, 0, sizeof(g_clients));
    for (i = 0; i < OUT_MAX_CLIENTS; i++) g_clients[i].fd = -1;

    g_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (g_listen_fd < 0) {
        printf("[OUT] socket() failed\n");
        return NULL;
    }
    setsockopt(g_listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(OUT_PORT);

    if (bind(g_listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("[OUT] bind(%d) failed\n", OUT_PORT);
        close(g_listen_fd);
        g_listen_fd = -1;
        return NULL;
    }
    if (listen(g_listen_fd, OUT_MAX_CLIENTS) < 0) {
        printf("[OUT] listen() failed\n");
        close(g_listen_fd);
        g_listen_fd = -1;
        return NULL;
    }

    printf("[OUT] cmd output server listening on %d\n", OUT_PORT);
    g_running = 1;

    while (g_running) {
        fd_set rfds;
        struct timeval tv;
        int maxfd = g_listen_fd;

        FD_ZERO(&rfds);
        FD_SET(g_listen_fd, &rfds);

        pthread_mutex_lock(&g_out_lock);
        for (i = 0; i < OUT_MAX_CLIENTS; i++) {
            if (g_clients[i].active) {
                FD_SET(g_clients[i].fd, &rfds);
                if (g_clients[i].fd > maxfd) maxfd = g_clients[i].fd;
            }
        }
        pthread_mutex_unlock(&g_out_lock);

        /* 超时设短一点，保证心跳发送及时 */
        tv.tv_sec = 0;
        tv.tv_usec = 100 * 1000;

        if (select(maxfd + 1, &rfds, NULL, NULL, &tv) > 0) {
            /* 新连接 */
            if (FD_ISSET(g_listen_fd, &rfds)) {
                cli_len = sizeof(cli);
                int nfd = accept(g_listen_fd, (struct sockaddr *)&cli, &cli_len);
                if (nfd >= 0) {
                    int slot = -1;
                    pthread_mutex_lock(&g_out_lock);
                    for (i = 0; i < OUT_MAX_CLIENTS; i++) {
                        if (!g_clients[i].active) { slot = i; break; }
                    }
                    if (slot >= 0) {
                        g_clients[slot].fd = nfd;
                        g_clients[slot].active = 1;
                        snprintf(g_clients[slot].addr, sizeof(g_clients[slot].addr),
                                 "%s:%d", inet_ntoa(cli.sin_addr), ntohs(cli.sin_port));
                        printf("[OUT] client connected: %s\n", g_clients[slot].addr);
                        /* 立即推一次当前指令 */
                        char init[16];
                        int n = snprintf(init, sizeof(init), "%s\n", g_cur_cmd);
                        send(nfd, init, n, 0);
                    } else {
                        printf("[OUT] no free slot, reject %s\n", inet_ntoa(cli.sin_addr));
                        close(nfd);
                    }
                    pthread_mutex_unlock(&g_out_lock);
                }
            }

            /* 已有客户端发来数据（心跳，内容忽略） */
            pthread_mutex_lock(&g_out_lock);
            for (i = 0; i < OUT_MAX_CLIENTS; i++) {
                if (g_clients[i].active && FD_ISSET(g_clients[i].fd, &rfds)) {
                    int n = recv(g_clients[i].fd, buf, sizeof(buf) - 1, 0);
                    if (n <= 0) {
                        pthread_mutex_unlock(&g_out_lock);
                        drop_client(i);
                        pthread_mutex_lock(&g_out_lock);
                    }
                }
            }
            pthread_mutex_unlock(&g_out_lock);
        }

        /* 心跳：每 OUT_HB_MS 重发当前指令，防执行端看门狗误触 */
        uint32_t now = out_now_ms();
        if ((now - g_last_send_ms) >= OUT_HB_MS) {
            g_last_send_ms = now;
            if (cmdout_client_count() > 0) {
                char msg[16];
                pthread_mutex_lock(&g_out_lock);
                int n = snprintf(msg, sizeof(msg), "%s\n", g_cur_cmd);
                pthread_mutex_unlock(&g_out_lock);
                broadcast(msg, n);
            }
        }
    }

    for (i = 0; i < OUT_MAX_CLIENTS; i++) {
        if (g_clients[i].active) close(g_clients[i].fd);
    }
    close(g_listen_fd);
    g_listen_fd = -1;
    printf("[OUT] stopped\n");
    return NULL;
}

int cmdout_start(void)
{
    pthread_t tid;
    if (pthread_create(&tid, NULL, out_thread, NULL) != 0) {
        printf("[OUT] pthread_create failed\n");
        return -1;
    }
    pthread_detach(tid);
    return 0;
}

void cmdout_stop(void)
{
    g_running = 0;
}
