#include "nna_app/include/sample_common_nna.h"
/* RT-Thread 提供毫秒级 tick，直接拿来用（避免自己换算出错）*/
extern unsigned long rt_tick_get_millisecond(void);

/* ★ 把模型内联进固件：避免 u-boot/RT-Thread 内存地址冲突
 *   模型放在源码目录 persondet.nbg，编译时嵌入 .rodata */
__asm__(
    ".section .data\n"
    ".global persondet_nbg_start\n"
    ".align 4\n"
    "persondet_nbg_start:\n"
    ".incbin \"sample_demo/nna_app/src/persondet.nbg\"\n"
    ".global persondet_nbg_end\n"
    "persondet_nbg_end:\n"
    ".previous\n"
);
extern const unsigned char persondet_nbg_start[];
extern const unsigned char persondet_nbg_end[];

static char *model_name = MODEL_TAG_DEMO_NNA_OBJ_DET;

static FH_UINT32 g_nna_grpid[MAX_GRP_NUM] = {0};
static FH_SINT32 g_nna_running[MAX_GRP_NUM] = {0};
static FH_SINT32 g_nna_stop[MAX_GRP_NUM] = {0};

/* ============================================================
 *  摇杆模块（D2/D3）：人形框 → 虚拟摇杆 (x, y) + 安全状态机
 * ============================================================ */
#define ROD_RANGE        1000      /* 摇杆输出范围 -1000 ~ +1000 */
#define ROD_DEAD_ZONE    100       /* 死区：小于此值视为 0（防抖） */
#define ROD_LOST_MS      1000      /* 人形丢失超过此时间 → 归零锁定 */
#define ROD_TIMEOUT_MS   30000     /* 无有效操作超时 → 锁定 */

/* D3：急停判定（手掌急停手势的替代实现）*/
#define ROD_STOP_SPEED   400       /* 中心横向速度阈值（归一化/秒 ×1000）——实测人手快挥约560 */
#define ROD_STOP_CONFIRM 1         /* 连续 N 帧判定（1=最快响应）*/
#define ROD_STOP_LOCK_MS 2000      /* 急停后锁定保持时间 */
#define ROD_SMOOTH_ALPHA  40       /* 摇杆平滑系数（0~100，越大越跟手）*/

/* D3：认证失败计数（等人脸库；目前用手形异常代替）*/
#define ROD_FAIL_LIMIT   3         /* 连续 N 次异常 → 锁定 */

typedef enum {
    ROD_LOCKED = 0,       /* 锁定：摇杆强制归零 */
    ROD_UNLOCKED = 1,     /* 解锁：允许摇杆输出 */
} rod_state_t;

typedef enum {
    ROD_LOCK_NONE = 0,
    ROD_LOCK_TARGET_LOST,   /* ① 人离开 >1s */
    ROD_LOCK_TIMEOUT,       /* ② 30s 无操作 */
    ROD_LOCK_EMERGENCY,     /* ③ 急停动作（快速横向挥手）*/
    ROD_LOCK_AUTH_FAIL,     /* ④ 认证失败 N 次（等人脸库）*/
} rod_lock_reason_t;

typedef struct {
    rod_state_t state;
    FH_SINT32   x;              /* -1000 ~ +1000，左右 */
    FH_SINT32   y;              /* -1000 ~ +1000，前后 */
    FH_SINT32   x_smooth;       /* 平滑后的输出（PC 端用这个）*/
    FH_SINT32   y_smooth;
    FH_SINT32   has_target;     /* 当前帧是否检测到人形 */
    FH_SINT32   box_cx, box_cy; /* 人形框中心（归一化 ×1000）*/
    FH_SINT32   box_x, box_y;   /* 人形框左上角（归一化 ×1000）*/
    FH_SINT32   box_w, box_h;
    FH_UINT64   last_seen_ms;   /* 最后检测到人形的时间 */
    FH_UINT64   last_action_ms; /* 最后有效操作时间 */
    FH_UINT64   lock_until_ms;  /* 强制锁定到此时刻（急停后）*/

    /* 急停判定用 */
    FH_SINT32   prev_cx;        /* 上一帧中心（归一化 ×1000）*/
    FH_UINT64   prev_cx_ms;
    FH_SINT32   stop_cnt;       /* 连续满足急停条件的帧数 */

    /* 认证/失败计数 */
    FH_SINT32   fail_cnt;
    rod_lock_reason_t lock_reason;
} rod_ctrl_t;

static rod_ctrl_t g_rod = { .state = ROD_LOCKED };

static FH_UINT64 rod_now_ms(void)
{
    return (FH_UINT64)rt_tick_get_millisecond();
}

static FH_SINT32 rod_clamp(FH_SINT32 v, FH_SINT32 lo, FH_SINT32 hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

/* 应用死区 */
static FH_SINT32 rod_deadzone(FH_SINT32 v)
{
    if (v > -ROD_DEAD_ZONE && v < ROD_DEAD_ZONE) return 0;
    return v;
}

/* 人形框 → 摇杆映射
 *   box 坐标为归一化 0.0~1.0（SDK 输出）
 *   x: 人形中心相对画面中心的偏移
 *   y: 人形框高度相对参考高度的比例（越大=越近=前进）
 */
/* 人形框 → 摇杆映射 + 4 个锁定条件 + 输出平滑 */
static void rod_update_from_det(NN_RESULT *nn_result, FH_FLOAT conf)
{
    FH_UINT64 now = rod_now_ms();
    FH_SINT32 found = 0;
    FH_SINT32 best = -1;
    FH_FLOAT best_conf = 0.0f;
    FH_SINT32 i;
    FH_SINT32 total;

    /* ★ 时序平滑：连续 N 帧投票，消除单帧漏检造成的闪烁 */
    static FH_SINT32 hit_cnt = 0;      /* 连续命中帧数 */
    static FH_SINT32 miss_cnt = 0;     /* 连续未命中帧数 */
#define DET_HIT_ON     2               /* 连续 2 帧命中 → 确认"有目标" */
#define DET_MISS_OFF   8               /* 连续 8 帧未命中 → 确认"无目标" */

    /* 防御：空指针 / 数量越界 */
    if (nn_result == NULL) return;
    total = nn_result->result_det.nn_result_num;
    if (total < 0) total = 0;
    if (total > BOX_MAX_NUM) total = BOX_MAX_NUM;

    /* 找置信度最高的人形框 */
    for (i = 0; i < total; i++)
    {
        FH_FLOAT c = nn_result->result_det.nn_result_info[i].conf;
        if (c >= conf && c > best_conf)
        {
            best_conf = c;
            best = i;
        }
    }

    /* ★ 时序平滑更新 */
    if (best >= 0 && best < total)
    {
        if (hit_cnt < 1000) hit_cnt++;
        miss_cnt = 0;
    }
    else
    {
        miss_cnt++;
        if (miss_cnt >= DET_MISS_OFF) hit_cnt = 0;
    }

    /* ★ 只有连续命中达到阈值才认为"有目标" */
    if (hit_cnt >= DET_HIT_ON && best >= 0 && best < total)
    {
        NN_RESULT_DET_INFO *d = &nn_result->result_det.nn_result_info[best];
        FH_FLOAT cx = d->box.x + d->box.w * 0.5f;
        FH_FLOAT h  = d->box.h;
        FH_SINT32 tx, ty;
        FH_SINT32 cx_n = (FH_SINT32)(cx * 1000.0f);

        found = 1;
        g_rod.has_target = 1;
        g_rod.last_seen_ms = now;

        /* ---- ③ 急停判定：中心横向速度超阈值 ---- */
        if (g_rod.prev_cx_ms && (now > g_rod.prev_cx_ms))
        {
            FH_SINT32 dx = cx_n - g_rod.prev_cx;
            FH_UINT64 dt = now - g_rod.prev_cx_ms;
            /* 速度 = |dx| / dt * 1000 （单位：归一化×1000 每秒）*/
            FH_SINT32 speed = (FH_SINT32)((FH_SINT32)(dx < 0 ? -dx : dx) * 1000 / (FH_SINT32)dt);

            if (speed > ROD_STOP_SPEED)
            {
                g_rod.stop_cnt++;
                if (g_rod.stop_cnt >= ROD_STOP_CONFIRM &&
                    g_rod.state == ROD_UNLOCKED)
                {
                    g_rod.state = ROD_LOCKED;
                    g_rod.lock_reason = ROD_LOCK_EMERGENCY;
                    g_rod.lock_until_ms = now + ROD_STOP_LOCK_MS;
                    g_rod.x = g_rod.y = 0;
                    g_rod.x_smooth = g_rod.y_smooth = 0;
                    g_rod.stop_cnt = 0;
                    g_rod.prev_cx = cx_n;
                    g_rod.prev_cx_ms = now;
                    SDK_PRT(model_name, "[ROD] LOCK (EMERGENCY speed=%d)\\n", (int)speed);
                    return;
                }
            }
            else
            {
                g_rod.stop_cnt = 0;   /* 速度回落，清零 */
            }
        }
        g_rod.prev_cx = cx_n;
        g_rod.prev_cx_ms = now;

        /* ---- x：中心偏差 [-0.5, +0.5] → [-1000, +1000] ---- */
        tx = (FH_SINT32)((cx - 0.5f) * 2.0f * (FH_FLOAT)ROD_RANGE);
        g_rod.x = rod_deadzone(rod_clamp(tx, -ROD_RANGE, ROD_RANGE));

        /* ---- y：人形框高度 0.15~0.85 → 0~1000（越近越大）---- */
        {
            FH_FLOAT hn = (h - 0.15f) / 0.70f;
            if (hn < 0.0f) hn = 0.0f;
            if (hn > 1.0f) hn = 1.0f;
            ty = (FH_SINT32)(hn * (FH_FLOAT)ROD_RANGE);
        }
        g_rod.y = rod_deadzone(rod_clamp(ty, 0, ROD_RANGE));

        g_rod.box_cx = cx_n;
        g_rod.box_x  = (FH_SINT32)(d->box.x * 1000.0f);
        g_rod.box_y  = (FH_SINT32)(d->box.y * 1000.0f);
        g_rod.box_w  = (FH_SINT32)(d->box.w * 1000.0f);
        g_rod.box_h  = (FH_SINT32)(h * 1000.0f);

        if (g_rod.x != 0 || g_rod.y != 0)
            g_rod.last_action_ms = now;

        /* 解锁（急停锁定期内不解锁）*/
        if (g_rod.state == ROD_LOCKED && now >= g_rod.lock_until_ms)
        {
            g_rod.state = ROD_UNLOCKED;
            g_rod.lock_reason = ROD_LOCK_NONE;
            g_rod.fail_cnt = 0;
            SDK_PRT(model_name, "[ROD] UNLOCK (person detected)\n");
        }
    }
    else
    {
        /* ★ 漏检帧：若仍在平滑保持期内（未连续 MISS_OFF 次），保持上次输出不归零 */
        if (hit_cnt > 0 && miss_cnt < DET_MISS_OFF)
        {
            /* 保持 g_rod.x / g_rod.y 不变，只维持状态 */
            g_rod.last_seen_ms = now;
            if (g_rod.state == ROD_LOCKED && now >= g_rod.lock_until_ms)
            {
                g_rod.state = ROD_UNLOCKED;
                g_rod.lock_reason = ROD_LOCK_NONE;
                g_rod.fail_cnt = 0;
            }
        }
        else
        {
            g_rod.has_target = 0;
            g_rod.prev_cx_ms = 0;      /* 失去目标，速度基准清零 */
            g_rod.stop_cnt = 0;
        }
    }

    /* ---- 安全状态机：4 个锁定条件 ---- */
    if (g_rod.state == ROD_UNLOCKED)
    {
        /* ① 人形丢失 > ROD_LOST_MS */
        if (!found && g_rod.last_seen_ms && (now - g_rod.last_seen_ms) > ROD_LOST_MS)
        {
            g_rod.state = ROD_LOCKED;
            g_rod.lock_reason = ROD_LOCK_TARGET_LOST;
            SDK_PRT(model_name, "[ROD] LOCK (target lost)\n");
        }
        /* ② 超时无操作 */
        else if (g_rod.last_action_ms && (now - g_rod.last_action_ms) > ROD_TIMEOUT_MS)
        {
            g_rod.state = ROD_LOCKED;
            g_rod.lock_reason = ROD_LOCK_TIMEOUT;
            SDK_PRT(model_name, "[ROD] LOCK (timeout)\n");
        }
        /* ④ 失败计数（等接人脸库；当前由外部调用 rod_report_auth_fail 触发）*/
        else if (g_rod.fail_cnt >= ROD_FAIL_LIMIT)
        {
            g_rod.state = ROD_LOCKED;
            g_rod.lock_reason = ROD_LOCK_AUTH_FAIL;
            SDK_PRT(model_name, "[ROD] LOCK (auth fail x%d)\n", (int)g_rod.fail_cnt);
        }
    }

    /* 锁定时强制归零 */
    if (g_rod.state == ROD_LOCKED)
    {
        g_rod.x = 0;
        g_rod.y = 0;
    }

    /* 输出平滑（一阶低通，避免抖动）*/
    g_rod.x_smooth = (g_rod.x * ROD_SMOOTH_ALPHA + g_rod.x_smooth * (100 - ROD_SMOOTH_ALPHA)) / 100;
    g_rod.y_smooth = (g_rod.y * ROD_SMOOTH_ALPHA + g_rod.y_smooth * (100 - ROD_SMOOTH_ALPHA)) / 100;

    {
        const char *rs = "none";
        switch (g_rod.lock_reason) {
        case ROD_LOCK_TARGET_LOST: rs = "lost";   break;
        case ROD_LOCK_TIMEOUT:     rs = "timeout"; break;
        case ROD_LOCK_EMERGENCY:   rs = "estop";  break;
        case ROD_LOCK_AUTH_FAIL:   rs = "authfail"; break;
        default: break;
        }
        SDK_PRT(model_name, "[ROD] %s x=%d y=%d xs=%d ys=%d find=%d rz=%s bb=%d,%d,%d,%d\n",
                g_rod.state == ROD_UNLOCKED ? "UNLOCK" : "LOCK",
                (int)g_rod.x, (int)g_rod.y,
                (int)g_rod.x_smooth, (int)g_rod.y_smooth,
                (int)found, rs,
                (int)g_rod.box_x, (int)g_rod.box_y,
                (int)g_rod.box_w, (int)g_rod.box_h);
    }
}

/* 外部可调用：上报一次认证失败（接人脸库后由认证模块调用）*/
__attribute__((unused))
static void rod_report_auth_fail(FH_SINT32 ok)
{
    if (ok) {
        g_rod.fail_cnt = 0;
    } else if (g_rod.fail_cnt < 99) {
        g_rod.fail_cnt++;
    }
}
/* ============================================================ */

#if defined(FH_OVERLAY_ENABLE)   /* 板端画框需 OSD 模块 */
static FH_SINT32 draw_box(FH_SINT32 grp_id, FH_SINT32 chn_id, NN_RESULT nn_result, FH_FLOAT conf)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;
    FH_UINT32 chn_w = 0;
    FH_UINT32 chn_h = 0;
    static OSD_GBOX gbox[BOX_MAX_NUM];
    FH_SINT32 i = 0;
    static NN_RESULT last_result = {0};

    if (nn_result.result_det.nn_result_num == 0)
    {
        for (i = 0; i < last_result.result_det.nn_result_num; i++)
        {
            gbox[i].enable = 0;

            ret = sample_set_gbox(grp_id, chn_id, &gbox[i]);
            SDK_FUNC_ERROR_CONTINUE(model_name, ret);
        }

        last_result.result_det.nn_result_num = 0;

        return FH_SDK_SUCCESS;
    }

    SDK_CHECK_MAX_VALID(model_name, nn_result.result_det.nn_result_num, BOX_MAX_NUM, "nn_result_num > BOX_MAX_NUM:%d\n", BOX_MAX_NUM);

    chn_w = get_vpu_chn_w(grp_id, chn_id);
    chn_h = get_vpu_chn_h(grp_id, chn_id);

    for (i = 0; i < nn_result.result_det.nn_result_num; i++)
    {
        gbox[i].gbox_id = i;
        gbox[i].color.fAlpha = 255;
        gbox[i].color.fRed = 255;
        gbox[i].color.fGreen = 0;
        gbox[i].color.fBlue = 0;
        gbox[i].x = nn_result.result_det.nn_result_info[i].box.x * chn_w;
        gbox[i].y = nn_result.result_det.nn_result_info[i].box.y * chn_h;
        gbox[i].w = nn_result.result_det.nn_result_info[i].box.w * chn_w;
        gbox[i].h = nn_result.result_det.nn_result_info[i].box.h * chn_h;

        if (nn_result.result_det.nn_result_info[i].conf < conf)
        {
            gbox[i].enable = 0;
        }
        else
        {
            gbox[i].enable = 1;
        }

        ret = sample_set_gbox(grp_id, chn_id, &gbox[i]);
        SDK_FUNC_ERROR_CONTINUE(model_name, ret);
    }

    if (last_result.result_det.nn_result_num > nn_result.result_det.nn_result_num)
    {
        for (; i < last_result.result_det.nn_result_num; i++)
        {
            gbox[i].enable = 0;

            ret = sample_set_gbox(grp_id, chn_id, &gbox[i]);
            SDK_FUNC_ERROR_CONTINUE(model_name, ret);
        }
    }

    last_result.result_det.nn_result_num = nn_result.result_det.nn_result_num;

    return ret;
}
#endif

FH_VOID *nna_obj_detect(FH_VOID *args)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;

    NN_HANDLE nn_handle = NULL;
    NN_TYPE nn_type;
    FH_CHAR model_path[256];
    VPU_INFO *vpu_info;
    VPU_CHN_INFO *vpu_chn_info;
    NN_INPUT_DATA nn_input_data;
    NN_RESULT nn_result;
    FH_SINT32 grp_id = *((FH_SINT32 *)args);
    FH_CHAR thread_name[20];
    FH_VIDEO_FRAME pstVpuframeinfo;
    FH_UINT32 handle_lock;
    FH_FLOAT fps = 0;
    FH_UINT64 time_start = 0;
    FH_UINT64 time_now = 0;
    FH_UINT64 time_bak = 0;
    FH_UINT64 time_lapse = 1000 / FH_NN_DET_FPS;
    INI_APP_CFG *ini_app_cfg = NULL;

    vpu_info = get_vpu_config(grp_id);

    if (vpu_info->enable)
    {
        vpu_chn_info = get_vpu_chn_config(grp_id, NN_CHN);
        if (!vpu_chn_info->enable)
        {
            SDK_ERR_PRT(model_name, "VPU[%d] CHN[%d] not enable !\n", grp_id, NN_CHN);
            goto Exit;
        }
    }
    else
    {
        SDK_ERR_PRT(model_name, "VPU GRP[%d] not enable !\n", grp_id);
        goto Exit;
    }

    ini_app_cfg = get_ini_app_cfg();
    if (ini_app_cfg)
    {
        if (!ini_app_cfg->nn_enable)
        {
            goto Exit;
        }
        time_lapse = 1000 / ini_app_cfg->nn_fps;
    }

#ifdef FH_NN_PERSON_DETECT_ENABLE
    nn_type = PERSON_DET;
    /* ★ 模型已内联进固件 .rodata，直接用符号地址，无需 u-boot 下载 */
    snprintf(model_path, sizeof(model_path), "mem:0x%08lx", (unsigned long)persondet_nbg_start);
#elif defined(FH_NN_C2D_ENABLE)
    nn_type = PERSON_VEHICLE_DET;
    strcpy(model_path, "./models/c2det.nbg");
#elif defined(FH_NN_FACE_DET_ENABLE)
    nn_type = FACE_DET;
    strcpy(model_path, "./models/facedet.nbg");
#elif defined(FH_NN_PVF_DET_ENABLE)
    nn_type = PERSON_VEHICLE_FIRE_DET;
    strcpy(model_path, "./models/c3det.nbg");
#elif defined(FH_APP_OPEN_LPD_ENABLE)
    nn_type = LPR_DET;
    strcpy(model_path, "./models/lpdet.nbg");
#else
    SDK_ERR_PRT(model_name, "nn_type NULL !\n");
    goto Exit;
#endif

    ret = nna_init(grp_id, &nn_handle, nn_type, model_path, vpu_chn_info->width, vpu_chn_info->height);
    SDK_FUNC_ERROR_GOTO(model_name, ret);

    memset(&nn_input_data, 0, sizeof(nn_input_data));
    nn_input_data.width = vpu_chn_info->width;
    nn_input_data.height = vpu_chn_info->height;
    nn_input_data.stride = vpu_chn_info->width;

    SDK_PRT(model_name, "nna_obj_detect[%d] Start!\n", grp_id);

    g_nna_running[grp_id] = 1;

    memset(thread_name, 0, sizeof(thread_name));
    sprintf(thread_name, "nna_obj_det_%d", grp_id);
    prctl(PR_SET_NAME, thread_name);
    sleep(1);
    time_start = time_bak = time_now = get_ms();
    while (!g_nna_stop[grp_id])
    {
        ret = lock_vpu_stream(grp_id, NN_CHN, &pstVpuframeinfo, 500, &handle_lock, 1);
        SDK_FUNC_ERROR_CONTINUE(model_name, ret);

        time_now = get_ms();
        if (time_now - time_bak > time_lapse)
        {
            time_bak += time_lapse;

            ret = getVPUStreamToNN(pstVpuframeinfo, &nn_input_data);
            if (!ret)
            {
                /* ★ 诊断：统计 NN 输入图像的像素，判断数据是否正常 */
                {
                    static int stat_cnt = 0;
                    if (stat_cnt < 3) {
                        unsigned char *px = (unsigned char *)nn_input_data.data.vbase;
                        unsigned int n = nn_input_data.width * nn_input_data.height * 3;
                        unsigned long sum = 0, sum2 = 0;
                        unsigned int mn = 255, mx = 0;
                        unsigned int i;
                        if (px && n > 0 && n < (4*1024*1024)) {
                            for (i = 0; i < n; i += 97) {   /* 抽样统计 */
                                unsigned int v = px[i];
                                sum += v; sum2 += v*v;
                                if (v < mn) mn = v;
                                if (v > mx) mx = v;
                            }
                            unsigned int cnt = (n + 96) / 97;
                            unsigned int avg = sum / (cnt ? cnt : 1);
                            SDK_PRT(model_name, "[DIAG] img w=%u h=%u strd=%u type=%d vbase=%p ppm: avg=%u min=%u max=%u\n",
                                    nn_input_data.width, nn_input_data.height, nn_input_data.stride,
                                    nn_input_data.imageType, px, avg, mn, mx);
                            SDK_PRT(model_name, "[DIAG] first16: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                                    px[0],px[1],px[2],px[3],px[4],px[5],px[6],px[7],
                                    px[8],px[9],px[10],px[11],px[12],px[13],px[14],px[15]);
                        } else {
                            SDK_PRT(model_name, "[DIAG] img ptr=%p n=%u ABNORMAL\n", px, n);
                        }
                        stat_cnt++;
                    }
                }

                ret = nna_get_obj_det_result(nn_handle, nn_input_data, &nn_result);
                SDK_FUNC_ERROR_CONTINUE(model_name, ret);
                fps++;

                /* ★ 诊断：打印检测框数量与置信度 */
                {
                    static int box_cnt = 0;
                    if (box_cnt < 100000) {
                        FH_UINT32 bn = nn_result.result_det.nn_result_num;
                        if (bn > 0) {
                            FH_FLOAT maxc = 0.0f;
                            FH_UINT32 bi;
                            for (bi = 0; bi < bn && bi < 20; bi++) {
                                if (nn_result.result_det.nn_result_info[bi].conf > maxc)
                                    maxc = nn_result.result_det.nn_result_info[bi].conf;
                            }
                            SDK_PRT(model_name, "[BOX] num=%u maxconf=%.3f\n", bn, maxc);
                        } else {
                            SDK_PRT(model_name, "[BOX] num=0\n");
                        }
                        box_cnt++;
                    }
                }

                ret = unlock_vpu_stream(grp_id, NN_CHN, handle_lock, &pstVpuframeinfo);
                SDK_FUNC_ERROR_CONTINUE(model_name, ret);

                /* 摇杆：人形框 → (x, y) + 安全状态机 */
                rod_update_from_det(&nn_result, 0.001f);

                /* 板端不画框（框由 PC 上位机绘制，省 OSD 依赖）*/
#if defined(FH_OVERLAY_ENABLE)
                ret = draw_box(grp_id, 0, nn_result, 0.001f);
                SDK_FUNC_ERROR_CONTINUE(model_name, ret);
#endif

                if (time_now - time_start > 10 * 1000)
                {
                    SDK_PRT(model_name, "NN[%d] Det FPS: %f\n", grp_id, fps / 10);
                    fps = 0;
                    time_start = time_now;
                }
            }
            else
            {
                ret = unlock_vpu_stream(grp_id, NN_CHN, handle_lock, &pstVpuframeinfo);
                SDK_FUNC_ERROR_CONTINUE(model_name, ret);
            }
        }
        else
        {
            ret = unlock_vpu_stream(grp_id, NN_CHN, handle_lock, &pstVpuframeinfo);
            SDK_FUNC_ERROR_CONTINUE(model_name, ret);
        }
    }

Exit:
    ret = nna_release(grp_id, &nn_handle);
    SDK_FUNC_ERROR_GOTO(model_name, ret);
    SDK_PRT(model_name, "nna_obj_detect[%d] End!\n", grp_id);

    g_nna_running[grp_id] = 0;

    return NULL;
}

FH_SINT32 sample_fh_nna_obj_detect_start(FH_SINT32 grp_id)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;
    pthread_t thread;
    pthread_attr_t attr;

    if (g_nna_running[grp_id])
    {
        SDK_PRT(model_name, "nna_obj_detect[%d] already running!\n", grp_id);
        return FH_SDK_SUCCESS;
    }

    g_nna_grpid[grp_id] = grp_id;
    g_nna_stop[grp_id] = 0;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
#ifdef __RTTHREAD_OS__
    pthread_attr_setstacksize(&attr, 256 * 1024);
#endif
    if (pthread_create(&thread, &attr, nna_obj_detect, &g_nna_grpid[grp_id]))
    {
        SDK_ERR_PRT(model_name, "pthread_create failed\n");
        goto Exit;
    }

    return ret;

Exit:
    return -1;
}

FH_SINT32 sample_fh_nna_obj_detect_stop(FH_SINT32 grp_id)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;

    if (g_nna_running[grp_id])
    {
        g_nna_stop[grp_id] = 1;

        while (g_nna_running[grp_id])
        {
            usleep(10);
        }
    }
    else
    {
        SDK_PRT(model_name, "nna_obj_detect[%d] not running!\n", grp_id);
    }

    return ret;
}
