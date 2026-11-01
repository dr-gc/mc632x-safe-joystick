#include "sample_common_demo.h"

static char *model_name = MODEL_TAG_DEMO;

/* 板端安全状态机：接收 PC 摇杆数据，自己判定状态并输出控制指令 */
extern int  ctrl_sm_start(void);
extern void ctrl_sm_stop(void);

FH_SINT32 sample_common_start_demo(FH_VOID)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;

    SDK_PRT(model_name, "Start Demo!\n");

    /* 启动板端安全状态机 */
    ctrl_sm_start();

#if defined(DUAL_CAMERA_ENABLE)
    ret = sample_dual_camera_start();
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_VISE_DEMO_ENABLE)
    ret = sample_fh_vise_demo_start(FH_APP_GRP_ID);
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_OVERLAY_ENABLE)
    ret = sample_fh_overlay_start(FH_APP_GRP_ID);
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(UVC_ENABLE)
    ret = sample_common_uvc_start();
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_VOU_ENABLE)
    ret = sample_fh_vou_start();
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_NN_DETECT_ENABLE)
#if defined(FH_NN_IVS_ENABLE)
    ret = sample_fh_nna_ivs_start(FH_APP_GRP_ID);
    SDK_FUNC_ERROR(model_name, ret);
#else
    ret = sample_fh_nna_obj_detect_start(FH_APP_GRP_ID);
    SDK_FUNC_ERROR(model_name, ret);

#endif
#endif

#if defined(FH_NN_AINR_ENABLE)
#if defined(FH_APP_USING_AINR_G0)
    ret = sample_fh_ainr_start(0);
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_APP_USING_AINR_G1)
    ret = sample_fh_ainr_start(1);
    SDK_FUNC_ERROR(model_name, ret);
#endif
#endif

#if defined(FH_WAVE_HAND_ENABLE)
    ret = sample_fh_nna_whd_start();
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_GESTURE_REC_ENABLE)
    ret = sample_fh_nna_gesture_rec_start(FH_APP_GRP_ID);
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_FACE_SNAP_ENABLE)
    ret = sample_fh_nna_face_snap_start(FH_APP_GRP_ID);
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_FACE_REC_ENABLE)
    ret = sample_fh_nna_face_rec_start(FH_APP_GRP_ID);
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_LP_REC_ENABLE)
    ret = sample_fh_nna_lp_rec_start(FH_APP_GRP_ID);
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_IVS_ENABLE)
    ret = sample_fh_ivs_start(FH_APP_GRP_ID);
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_MD_CD_ENABLE)
    ret = sample_fh_md_cd_start(FH_APP_GRP_ID);
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_ISP_DEMO_ENABLE)
    ret = sample_fh_isp_demo_start(FH_APP_GRP_ID);
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_VENC_DEMO_ENABLE)
    ret = sample_fh_venc_start(FH_APP_GRP_ID);
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_AF_ENABLE)
    ret = sample_fh_af_start(FH_APP_GRP_ID);
    SDK_FUNC_ERROR(model_name, ret);
#endif

    return ret;
}

FH_SINT32 sample_common_stop_demo(FH_VOID)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;

    SDK_PRT(model_name, "Demo exit!\n");
#if defined(FH_AF_ENABLE)
    SDK_PRT(model_name, "sample_fh_af_stop!\n");
    ret = sample_fh_af_stop(FH_APP_GRP_ID);
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_VENC_DEMO_ENABLE)
    SDK_PRT(model_name, "sample_fh_venc_stop!\n");
    ret = sample_fh_venc_stop(FH_APP_GRP_ID);
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_ISP_DEMO_ENABLE)
    SDK_PRT(model_name, "sample_fh_isp_demo_stop!\n");
    ret = sample_fh_isp_demo_stop(FH_APP_GRP_ID);
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_MD_CD_ENABLE)
    SDK_PRT(model_name, "sample_fh_md_cd_stop!\n");
    ret = sample_fh_md_cd_stop(FH_APP_GRP_ID);
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_IVS_ENABLE)
    SDK_PRT(model_name, "sample_fh_ivs_stop!\n");
    ret = sample_fh_ivs_stop(FH_APP_GRP_ID);
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_FACE_REC_ENABLE)
    SDK_PRT(model_name, "sample_fh_nna_face_rec_stop!\n");
    ret = sample_fh_nna_face_rec_stop(FH_APP_GRP_ID);
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_FACE_SNAP_ENABLE)
    SDK_PRT(model_name, "sample_fh_nna_face_snap_stop!\n");
    ret = sample_fh_nna_face_snap_stop(FH_APP_GRP_ID);
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_LP_REC_ENABLE)
    SDK_PRT(model_name, "sample_fh_nna_lp_rec_stop!\n");
    ret = sample_fh_nna_lp_rec_stop(FH_APP_GRP_ID);
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_GESTURE_REC_ENABLE)
    SDK_PRT(model_name, "sample_fh_nna_gesture_rec_stop!\n");
    ret = sample_fh_nna_gesture_rec_stop(FH_APP_GRP_ID);
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_APP_OPEN_WAVE_HAND_DETECT)
    SDK_PRT(model_name, "sample_fh_nna_whd_stop!\n");
    ret = sample_fh_nna_whd_stop();
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_NN_AINR_ENABLE)
#if defined(FH_APP_USING_AINR_G0)
    ret = sample_fh_ainr_stop(0);
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_APP_USING_AINR_G1)
    ret = sample_fh_ainr_stop(1);
    SDK_FUNC_ERROR(model_name, ret);
#endif
#endif

#if defined(FH_NN_DETECT_ENABLE)
#if defined(FH_NN_IVS_ENABLE)
    SDK_PRT(model_name, "sample_fh_nna_ivs_stop!\n");
    ret = sample_fh_nna_ivs_stop(FH_APP_GRP_ID);
    SDK_FUNC_ERROR(model_name, ret);
#else
    SDK_PRT(model_name, "sample_fh_nna_obj_detect_stop!\n");
    ret = sample_fh_nna_obj_detect_stop(FH_APP_GRP_ID);
    SDK_FUNC_ERROR(model_name, ret);
#endif
#endif

#if defined(FHB_VOU_ENABLE)
    SDK_PRT(model_name, "sample_fh_vou_stop!\n");
    ret = sample_fh_vou_stop();
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(UVC_ENABLE)
    SDK_PRT(model_name, "sample_common_uvc_stop!\n");
    ret = sample_common_uvc_stop();
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_OVERLAY_ENABLE)
    SDK_PRT(model_name, "sample_fh_overlay_stop!\n");
    ret = sample_fh_overlay_stop(FH_APP_GRP_ID);
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(FH_VISE_DEMO_ENABLE)
    SDK_PRT(model_name, "sample_fh_vise_demo_stop!\n");
    ret = sample_fh_vise_demo_stop(FH_APP_GRP_ID);
    SDK_FUNC_ERROR(model_name, ret);
#endif

#if defined(DUAL_CAMERA_ENABLE)
    SDK_PRT(model_name, "sample_dual_camera_stop!\n");
    ret = sample_dual_camera_stop();
    SDK_FUNC_ERROR(model_name, ret);
#endif

    return ret;
}