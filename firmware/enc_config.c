#include "enc_config.h"
#include "jpeg_config.h"

static char *model_name = MODEL_TAG_ENC_CONFIG;

static ENC_INFO g_enc_chn_infos[MAX_GRP_NUM][MAX_VPU_CHN_NUM] = {
    {
        {
            .enable = 0,
#if defined(CH0_BIND_ENC_G0) && defined(CH0_ENC_TYPE_G0)
            .enable = 1,
            .channel = 0,
            .width = CH0_WIDTH_G0,
            .height = CH0_HEIGHT_G0,
#if (CH0_MAX_WIDTH_G0 > CH0_WIDTH_G0)
            .max_width = CH0_MAX_WIDTH_G0,
#else
            .max_width = CH0_WIDTH_G0,
#endif
#if (CH0_MAX_HEIGHT_G0 > CH0_HEIGHT_G0)
            .max_height = CH0_MAX_HEIGHT_G0,
#else
            .max_height = CH0_HEIGHT_G0,
#endif
            .frame_count = CH0_FRAME_COUNT_G0,
            .frame_time = CH0_FRAME_TIME_G0,
            .bps = CH0_BIT_RATE_G0 * 1024,
            .enc_type = CH0_ENC_TYPE_G0,
            .rc_type = CH0_RC_TYPE_G0,
            .breath_on = CH0_BREATH_OPEN_G0,
#endif /*CH0_BIND_ENC_G0*/
        },

        {
            .enable = 0,
#if defined(CH1_BIND_ENC_G0) && defined(CH1_ENC_TYPE_G0)
            .enable = 1,
            .channel = 1,
            .width = CH1_WIDTH_G0,
            .height = CH1_HEIGHT_G0,
#if (CH1_MAX_WIDTH_G0 > CH1_WIDTH_G0)
            .max_width = CH1_MAX_WIDTH_G0,
#else
            .max_width = CH1_WIDTH_G0,
#endif
#if (CH1_MAX_HEIGHT_G0 > CH1_HEIGHT_G0)
            .max_height = CH1_MAX_HEIGHT_G0,
#else
            .max_height = CH1_HEIGHT_G0,
#endif
            .frame_count = CH1_FRAME_COUNT_G0,
            .frame_time = CH1_FRAME_TIME_G0,
            .bps = CH1_BIT_RATE_G0 * 1024,
            .enc_type = CH1_ENC_TYPE_G0,
            .rc_type = CH1_RC_TYPE_G0,
            .breath_on = CH1_BREATH_OPEN_G0,
#endif /*CH1_BIND_ENC_G0*/
        },

        {
            .enable = 0,
#if defined(CH2_BIND_ENC_G0) && defined(CH2_ENC_TYPE_G0)
            .enable = 1,
            .channel = 2,
            .width = CH2_WIDTH_G0,
            .height = CH2_HEIGHT_G0,
#if (CH2_MAX_WIDTH_G0 > CH2_WIDTH_G0)
            .max_width = CH2_MAX_WIDTH_G0,
#else
            .max_width = CH2_WIDTH_G0,
#endif
#if (CH2_MAX_HEIGHT_G0 > CH2_HEIGHT_G0)
            .max_height = CH2_MAX_HEIGHT_G0,
#else
            .max_height = CH2_HEIGHT_G0,
#endif
            .frame_count = CH2_FRAME_COUNT_G0,
            .frame_time = CH2_FRAME_TIME_G0,
            .bps = CH2_BIT_RATE_G0 * 1024,
            .enc_type = CH2_ENC_TYPE_G0,
            .rc_type = CH2_RC_TYPE_G0,
            .breath_on = CH2_BREATH_OPEN_G0,
#endif /*CH2_BIND_ENC_G0*/
        },
    }, /* G0 */
    {
        {
            .enable = 0,
#if defined(CH0_BIND_ENC_G1) && defined(CH0_ENC_TYPE_G1)
            .enable = 1,
            .channel = 0,
            .width = CH0_WIDTH_G1,
            .height = CH0_HEIGHT_G1,
#if (CH0_MAX_WIDTH_G1 > CH0_WIDTH_G1)
            .max_width = CH0_MAX_WIDTH_G1,
#else
            .max_width = CH0_WIDTH_G1,
#endif
#if (CH0_MAX_HEIGHT_G1 > CH0_HEIGHT_G1)
            .max_height = CH0_MAX_HEIGHT_G1,
#else
            .max_height = CH0_HEIGHT_G1,
#endif
            .frame_count = CH0_FRAME_COUNT_G1,
            .frame_time = CH0_FRAME_TIME_G1,
            .bps = CH0_BIT_RATE_G1 * 1024,
            .enc_type = CH0_ENC_TYPE_G1,
            .rc_type = CH0_RC_TYPE_G1,
            .breath_on = CH0_BREATH_OPEN_G1,
#endif /*CH0_BIND_ENC_G1*/
        },

        {
            .enable = 0,
#if defined(CH1_BIND_ENC_G1) && defined(CH1_ENC_TYPE_G1)
            .enable = 1,
            .channel = 1,
            .width = CH1_WIDTH_G1,
            .height = CH1_HEIGHT_G1,
#if (CH1_MAX_WIDTH_G1 > CH1_WIDTH_G1)
            .max_width = CH1_MAX_WIDTH_G1,
#else
            .max_width = CH1_WIDTH_G1,
#endif
#if (CH1_MAX_HEIGHT_G1 > CH1_HEIGHT_G1)
            .max_height = CH1_MAX_HEIGHT_G1,
#else
            .max_height = CH1_HEIGHT_G1,
#endif
            .frame_count = CH1_FRAME_COUNT_G1,
            .frame_time = CH1_FRAME_TIME_G1,
            .bps = CH1_BIT_RATE_G1 * 1024,
            .enc_type = CH1_ENC_TYPE_G1,
            .rc_type = CH1_RC_TYPE_G1,
            .breath_on = CH1_BREATH_OPEN_G1,
#endif /*CH1_BIND_ENC_G1*/
        },

        {
            .enable = 0,
#if defined(CH2_BIND_ENC_G1) && defined(CH2_ENC_TYPE_G1)
            .enable = 1,
            .channel = 2,
            .width = CH2_WIDTH_G1,
            .height = CH2_HEIGHT_G1,
#if (CH2_MAX_WIDTH_G1 > CH2_WIDTH_G1)
            .max_width = CH2_MAX_WIDTH_G1,
#else
            .max_width = CH2_WIDTH_G1,
#endif
#if (CH2_MAX_HEIGHT_G1 > CH2_HEIGHT_G1)
            .max_height = CH2_MAX_HEIGHT_G1,
#else
            .max_height = CH2_HEIGHT_G1,
#endif
            .frame_count = CH2_FRAME_COUNT_G1,
            .frame_time = CH2_FRAME_TIME_G1,
            .bps = CH2_BIT_RATE_G1 * 1024,
            .enc_type = CH2_ENC_TYPE_G1,
            .rc_type = CH2_RC_TYPE_G1,
            .breath_on = CH2_BREATH_OPEN_G1,
#endif /*CH2_BIND_ENC_G1*/
        },
    }, /* G1 */
};

static FH_SINT32 set_264_rc(FH_SINT32 grp_id, FH_SINT32 chn_id, FH_VENC_CHN_CONFIG *cfg_param)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;

    switch (g_enc_chn_infos[grp_id][chn_id].rc_type)
    {
    case FH_RC_H264_VBR:
        cfg_param->rc_attr.rc_type = FH_RC_H264_VBR;
        cfg_param->rc_attr.h264_vbr.bitrate = g_enc_chn_infos[grp_id][chn_id].bps;
        cfg_param->rc_attr.h264_vbr.init_qp = 35;
        cfg_param->rc_attr.h264_vbr.ImaxQP = VIDEO_IMAXQP;
        cfg_param->rc_attr.h264_vbr.IminQP = VIDEO_IMINQP;
        cfg_param->rc_attr.h264_vbr.PmaxQP = VIDEO_PMAXQP;
        cfg_param->rc_attr.h264_vbr.PminQP = VIDEO_PMINQP;
        cfg_param->rc_attr.h264_vbr.FrameRate.frame_count = g_enc_chn_infos[grp_id][chn_id].frame_count;
        cfg_param->rc_attr.h264_vbr.FrameRate.frame_time = g_enc_chn_infos[grp_id][chn_id].frame_time;
        cfg_param->rc_attr.h264_vbr.maxrate_percent = 200;
        cfg_param->rc_attr.h264_vbr.IFrmMaxBits = 0;
        if (g_enc_chn_infos[grp_id][chn_id].breath_on)
        {
            cfg_param->rc_attr.h264_vbr.IP_QPDelta = H264_BREATH_IP_QPDELTA;
            cfg_param->rc_attr.h264_vbr.I_BitProp = H264_BREATH_I_BITPROP;
            cfg_param->rc_attr.h264_vbr.P_BitProp = H264_BREATH_P_BITPROP;
        }
        else
        {
            cfg_param->rc_attr.h264_vbr.IP_QPDelta = 3;
            cfg_param->rc_attr.h264_vbr.I_BitProp = 5;
            cfg_param->rc_attr.h264_vbr.P_BitProp = 1;
        }
        cfg_param->rc_attr.h264_vbr.fluctuate_level = 0;
        break;
    case FH_RC_H264_CBR:
        cfg_param->rc_attr.rc_type = FH_RC_H264_CBR;
        cfg_param->rc_attr.h264_cbr.bitrate = g_enc_chn_infos[grp_id][chn_id].bps;
        cfg_param->rc_attr.h264_cbr.init_qp = 35;
        cfg_param->rc_attr.h264_cbr.FrameRate.frame_count = g_enc_chn_infos[grp_id][chn_id].frame_count;
        cfg_param->rc_attr.h264_cbr.FrameRate.frame_time = g_enc_chn_infos[grp_id][chn_id].frame_time;
        cfg_param->rc_attr.h264_cbr.maxrate_percent = 200;
        cfg_param->rc_attr.h264_cbr.IFrmMaxBits = 0;
        if (g_enc_chn_infos[grp_id][chn_id].breath_on)
        {
            cfg_param->rc_attr.h264_cbr.IP_QPDelta = H264_BREATH_IP_QPDELTA;
            cfg_param->rc_attr.h264_cbr.I_BitProp = H264_BREATH_I_BITPROP;
            cfg_param->rc_attr.h264_cbr.P_BitProp = H264_BREATH_P_BITPROP;
        }
        else
        {
            cfg_param->rc_attr.h264_cbr.IP_QPDelta = 3;
            cfg_param->rc_attr.h264_cbr.I_BitProp = 5;
            cfg_param->rc_attr.h264_cbr.P_BitProp = 1;
        }

        cfg_param->rc_attr.h264_cbr.fluctuate_level = 0;
        break;
    case FH_RC_H264_FIXQP:
        cfg_param->rc_attr.rc_type = FH_RC_H264_FIXQP;
        cfg_param->rc_attr.h264_fixqp.iqp = 28;
        cfg_param->rc_attr.h264_fixqp.pqp = 28;
        cfg_param->rc_attr.h264_fixqp.FrameRate.frame_count = g_enc_chn_infos[grp_id][chn_id].frame_count;
        cfg_param->rc_attr.h264_fixqp.FrameRate.frame_time = g_enc_chn_infos[grp_id][chn_id].frame_time;
        break;
    case FH_RC_H264_AVBR:
        cfg_param->rc_attr.rc_type = FH_RC_H264_AVBR;
        cfg_param->rc_attr.h264_avbr.bitrate = g_enc_chn_infos[grp_id][chn_id].bps;
        cfg_param->rc_attr.h264_avbr.init_qp = 35;
        cfg_param->rc_attr.h264_avbr.ImaxQP = 50;
        cfg_param->rc_attr.h264_avbr.IminQP = 28;
        cfg_param->rc_attr.h264_avbr.PmaxQP = 50;
        cfg_param->rc_attr.h264_avbr.PminQP = 28;
        cfg_param->rc_attr.h264_avbr.FrameRate.frame_count = g_enc_chn_infos[grp_id][chn_id].frame_count;
        cfg_param->rc_attr.h264_avbr.FrameRate.frame_time = g_enc_chn_infos[grp_id][chn_id].frame_time;
        cfg_param->rc_attr.h264_avbr.maxrate_percent = 200;
        cfg_param->rc_attr.h264_avbr.IFrmMaxBits = 0;
        if (g_enc_chn_infos[grp_id][chn_id].breath_on)
        {
            cfg_param->rc_attr.h264_avbr.IP_QPDelta = H264_BREATH_IP_QPDELTA;
            cfg_param->rc_attr.h264_avbr.I_BitProp = H264_BREATH_I_BITPROP;
            cfg_param->rc_attr.h264_avbr.P_BitProp = H264_BREATH_P_BITPROP;
        }
        else
        {
            cfg_param->rc_attr.h264_avbr.IP_QPDelta = 3;
            cfg_param->rc_attr.h264_avbr.I_BitProp = 5;
            cfg_param->rc_attr.h264_avbr.P_BitProp = 1;
        }
        cfg_param->rc_attr.h264_avbr.fluctuate_level = 0;
        cfg_param->rc_attr.h264_avbr.stillrate_percent = 30;
        cfg_param->rc_attr.h264_avbr.maxstillqp = 34;
        break;
    case FH_RC_H264_CVBR:
        cfg_param->rc_attr.rc_type = FH_RC_H264_CVBR;
        cfg_param->rc_attr.h264_cvbr.ltbitrate = g_enc_chn_infos[grp_id][chn_id].bps;
        cfg_param->rc_attr.h264_cvbr.init_qp = 35;
        cfg_param->rc_attr.h264_cvbr.ImaxQP = 50;
        cfg_param->rc_attr.h264_cvbr.IminQP = 28;
        cfg_param->rc_attr.h264_cvbr.PmaxQP = 50;
        cfg_param->rc_attr.h264_cvbr.PminQP = 28;
        cfg_param->rc_attr.h264_cvbr.FrameRate.frame_count = g_enc_chn_infos[grp_id][chn_id].frame_count;
        cfg_param->rc_attr.h264_cvbr.FrameRate.frame_time = g_enc_chn_infos[grp_id][chn_id].frame_time;
        cfg_param->rc_attr.h264_cvbr.maxrate_percent = 200;
        cfg_param->rc_attr.h264_cvbr.IFrmMaxBits = 0;
        if (g_enc_chn_infos[grp_id][chn_id].breath_on)
        {
            cfg_param->rc_attr.h264_cvbr.IP_QPDelta = H264_BREATH_IP_QPDELTA;
            cfg_param->rc_attr.h264_cvbr.I_BitProp = H264_BREATH_I_BITPROP;
            cfg_param->rc_attr.h264_cvbr.P_BitProp = H264_BREATH_P_BITPROP;
        }
        else
        {
            cfg_param->rc_attr.h264_cvbr.IP_QPDelta = 3;
            cfg_param->rc_attr.h264_cvbr.I_BitProp = 5;
            cfg_param->rc_attr.h264_cvbr.P_BitProp = 1;
        }
        cfg_param->rc_attr.h264_cvbr.fluctuate_level = 0;
        cfg_param->rc_attr.h264_cvbr.stbitrate = g_enc_chn_infos[grp_id][chn_id].bps * 2;
        cfg_param->rc_attr.h264_cvbr.acceptable_qp = 32;
        break;

    case FH_RC_H264_QVBR:
        cfg_param->rc_attr.rc_type = FH_RC_H264_QVBR;
        cfg_param->rc_attr.h264_qvbr.bitrate = g_enc_chn_infos[grp_id][chn_id].bps;
        cfg_param->rc_attr.h264_qvbr.init_qp = 35;
        cfg_param->rc_attr.h264_qvbr.ImaxQP = 50;
        cfg_param->rc_attr.h264_qvbr.IminQP = 28;
        cfg_param->rc_attr.h264_qvbr.PmaxQP = 50;
        cfg_param->rc_attr.h264_qvbr.PminQP = 28;
        cfg_param->rc_attr.h264_qvbr.FrameRate.frame_count = g_enc_chn_infos[grp_id][chn_id].frame_count;
        cfg_param->rc_attr.h264_qvbr.FrameRate.frame_time = g_enc_chn_infos[grp_id][chn_id].frame_time;
        cfg_param->rc_attr.h264_qvbr.maxrate_percent = 200;
        cfg_param->rc_attr.h264_qvbr.IFrmMaxBits = 0;
        if (g_enc_chn_infos[grp_id][chn_id].breath_on)
        {
            cfg_param->rc_attr.h264_qvbr.IP_QPDelta = H264_BREATH_IP_QPDELTA;
            cfg_param->rc_attr.h264_qvbr.I_BitProp = H264_BREATH_I_BITPROP;
            cfg_param->rc_attr.h264_qvbr.P_BitProp = H264_BREATH_P_BITPROP;
        }
        else
        {
            cfg_param->rc_attr.h264_qvbr.IP_QPDelta = 3;
            cfg_param->rc_attr.h264_qvbr.I_BitProp = 5;
            cfg_param->rc_attr.h264_qvbr.P_BitProp = 1;
        }
        cfg_param->rc_attr.h264_qvbr.fluctuate_level = 0;
        cfg_param->rc_attr.h264_qvbr.minrate_percent = 30;
        cfg_param->rc_attr.h264_qvbr.psnr_ul = 3200;
        cfg_param->rc_attr.h264_qvbr.psnr_ll = 2900;
        break;

    case FH_RC_H264_MVBR:
        cfg_param->rc_attr.rc_type = FH_RC_H264_MVBR;
        cfg_param->rc_attr.h264_mvbr.init_qp = 35;
        cfg_param->rc_attr.h264_mvbr.stbitrate = g_enc_chn_infos[grp_id][chn_id].bps * 2;
        cfg_param->rc_attr.h264_mvbr.ltbitrate = g_enc_chn_infos[grp_id][chn_id].bps;
        cfg_param->rc_attr.h264_mvbr.maxrate_percent = 200;
        cfg_param->rc_attr.h264_mvbr.IminQP = 28;
        cfg_param->rc_attr.h264_mvbr.ImaxQP = 50;
        cfg_param->rc_attr.h264_mvbr.PminQP = 28;
        cfg_param->rc_attr.h264_mvbr.PmaxQP = 50;
        cfg_param->rc_attr.h264_mvbr.FrameRate.frame_count = g_enc_chn_infos[grp_id][chn_id].frame_count;
        cfg_param->rc_attr.h264_mvbr.FrameRate.frame_time = g_enc_chn_infos[grp_id][chn_id].frame_time;
        cfg_param->rc_attr.h264_mvbr.IFrmMaxBits = 0;
        if (g_enc_chn_infos[grp_id][chn_id].breath_on)
        {
            cfg_param->rc_attr.h264_mvbr.IP_QPDelta = H264_BREATH_IP_QPDELTA;
            cfg_param->rc_attr.h264_mvbr.I_BitProp = H264_BREATH_I_BITPROP;
            cfg_param->rc_attr.h264_mvbr.P_BitProp = H264_BREATH_P_BITPROP;
        }
        else
        {
            cfg_param->rc_attr.h264_mvbr.IP_QPDelta = 3;
            cfg_param->rc_attr.h264_mvbr.I_BitProp = 5;
            cfg_param->rc_attr.h264_mvbr.P_BitProp = 1;
        }
        cfg_param->rc_attr.h264_mvbr.fluctuate_level = 0;
        cfg_param->rc_attr.h264_mvbr.acceptable_qp = 32;
        cfg_param->rc_attr.h264_mvbr.stillrate_percent = 10;
        cfg_param->rc_attr.h264_mvbr.maxstillqp = 36;
        break;
    default:
        ret = -1;
        break;
    }
    SDK_FUNC_ERROR(model_name, ret);

    ret = FH_VENC_SetChnAttr(grp_id * MAX_VPU_CHN_NUM + chn_id, cfg_param);
    SDK_FUNC_ERROR(model_name, ret);

    return ret;
}

static FH_SINT32 set_265_rc(FH_SINT32 grp_id, FH_SINT32 chn_id, FH_VENC_CHN_CONFIG *cfg_param)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;

    switch (g_enc_chn_infos[grp_id][chn_id].rc_type)
    {
    case FH_RC_H265_VBR:
        cfg_param->rc_attr.rc_type = FH_RC_H265_VBR;
        cfg_param->rc_attr.h265_vbr.bitrate = g_enc_chn_infos[grp_id][chn_id].bps;
        cfg_param->rc_attr.h265_vbr.init_qp = 35;
        cfg_param->rc_attr.h265_vbr.ImaxQP = 51;
        cfg_param->rc_attr.h265_vbr.IminQP = 28;
        cfg_param->rc_attr.h265_vbr.PmaxQP = 51;
        cfg_param->rc_attr.h265_vbr.PminQP = 28;
        cfg_param->rc_attr.h265_vbr.FrameRate.frame_count = g_enc_chn_infos[grp_id][chn_id].frame_count;
        cfg_param->rc_attr.h265_vbr.FrameRate.frame_time = g_enc_chn_infos[grp_id][chn_id].frame_time;
        cfg_param->rc_attr.h265_vbr.maxrate_percent = 200;
        cfg_param->rc_attr.h265_vbr.IFrmMaxBits = 0;
        if (g_enc_chn_infos[grp_id][chn_id].breath_on)
        {
            cfg_param->rc_attr.h265_vbr.IP_QPDelta = H265_BREATH_IP_QPDELTA;
            cfg_param->rc_attr.h265_vbr.I_BitProp = H265_BREATH_I_BITPROP;
            cfg_param->rc_attr.h265_vbr.P_BitProp = H265_BREATH_P_BITPROP;
        }
        else
        {
            cfg_param->rc_attr.h265_vbr.IP_QPDelta = 3;
            cfg_param->rc_attr.h265_vbr.I_BitProp = 5;
            cfg_param->rc_attr.h265_vbr.P_BitProp = 1;
        }
        cfg_param->rc_attr.h265_vbr.fluctuate_level = 0;
        break;
    case FH_RC_H265_CBR:
        cfg_param->rc_attr.rc_type = FH_RC_H265_CBR;
        cfg_param->rc_attr.h265_cbr.bitrate = g_enc_chn_infos[grp_id][chn_id].bps;
        cfg_param->rc_attr.h265_cbr.init_qp = 35;
        cfg_param->rc_attr.h265_cbr.FrameRate.frame_count = g_enc_chn_infos[grp_id][chn_id].frame_count;
        cfg_param->rc_attr.h265_cbr.FrameRate.frame_time = g_enc_chn_infos[grp_id][chn_id].frame_time;
        cfg_param->rc_attr.h265_cbr.maxrate_percent = 200;
        cfg_param->rc_attr.h265_cbr.IFrmMaxBits = 0;
        if (g_enc_chn_infos[grp_id][chn_id].breath_on)
        {
            cfg_param->rc_attr.h265_cbr.IP_QPDelta = H265_BREATH_IP_QPDELTA;
            cfg_param->rc_attr.h265_cbr.I_BitProp = H265_BREATH_I_BITPROP;
            cfg_param->rc_attr.h265_cbr.P_BitProp = H265_BREATH_P_BITPROP;
        }
        else
        {
            cfg_param->rc_attr.h265_cbr.IP_QPDelta = 3;
            cfg_param->rc_attr.h265_cbr.I_BitProp = 5;
            cfg_param->rc_attr.h265_cbr.P_BitProp = 1;
        }
        cfg_param->rc_attr.h265_cbr.fluctuate_level = 0;
        break;
    case FH_RC_H265_FIXQP:
        cfg_param->rc_attr.rc_type = FH_RC_H265_FIXQP;
        cfg_param->rc_attr.h265_fixqp.iqp = 28;
        cfg_param->rc_attr.h265_fixqp.qp = 28;
        cfg_param->rc_attr.h265_fixqp.FrameRate.frame_count = g_enc_chn_infos[grp_id][chn_id].frame_count;
        cfg_param->rc_attr.h265_fixqp.FrameRate.frame_time = g_enc_chn_infos[grp_id][chn_id].frame_time;
        break;
    case FH_RC_H265_AVBR:
        cfg_param->rc_attr.rc_type = FH_RC_H265_AVBR;
        cfg_param->rc_attr.h265_avbr.bitrate = g_enc_chn_infos[grp_id][chn_id].bps;
        cfg_param->rc_attr.h265_avbr.init_qp = 35;
        cfg_param->rc_attr.h265_avbr.ImaxQP = 50;
        cfg_param->rc_attr.h265_avbr.IminQP = 28;
        cfg_param->rc_attr.h265_avbr.PmaxQP = 50;
        cfg_param->rc_attr.h265_avbr.PminQP = 28;
        cfg_param->rc_attr.h265_avbr.FrameRate.frame_count = g_enc_chn_infos[grp_id][chn_id].frame_count;
        cfg_param->rc_attr.h265_avbr.FrameRate.frame_time = g_enc_chn_infos[grp_id][chn_id].frame_time;
        cfg_param->rc_attr.h265_avbr.maxrate_percent = 200;
        cfg_param->rc_attr.h265_avbr.IFrmMaxBits = 0;
        if (g_enc_chn_infos[grp_id][chn_id].breath_on)
        {
            cfg_param->rc_attr.h265_avbr.IP_QPDelta = H265_BREATH_IP_QPDELTA;
            cfg_param->rc_attr.h265_avbr.I_BitProp = H265_BREATH_I_BITPROP;
            cfg_param->rc_attr.h265_avbr.P_BitProp = H265_BREATH_P_BITPROP;
        }
        else
        {
            cfg_param->rc_attr.h265_avbr.IP_QPDelta = 3;
            cfg_param->rc_attr.h265_avbr.I_BitProp = 5;
            cfg_param->rc_attr.h265_avbr.P_BitProp = 1;
        }
        cfg_param->rc_attr.h265_avbr.fluctuate_level = 0;
        cfg_param->rc_attr.h265_avbr.stillrate_percent = 30;
        cfg_param->rc_attr.h265_avbr.maxstillqp = 34;
        break;
    case FH_RC_H265_CVBR:
        cfg_param->rc_attr.rc_type = FH_RC_H265_CVBR;
        cfg_param->rc_attr.h265_cvbr.ltbitrate = g_enc_chn_infos[grp_id][chn_id].bps;
        cfg_param->rc_attr.h265_cvbr.init_qp = 35;
        cfg_param->rc_attr.h265_cvbr.ImaxQP = 50;
        cfg_param->rc_attr.h265_cvbr.IminQP = 28;
        cfg_param->rc_attr.h265_cvbr.PmaxQP = 50;
        cfg_param->rc_attr.h265_cvbr.PminQP = 28;
        cfg_param->rc_attr.h265_cvbr.FrameRate.frame_count = g_enc_chn_infos[grp_id][chn_id].frame_count;
        cfg_param->rc_attr.h265_cvbr.FrameRate.frame_time = g_enc_chn_infos[grp_id][chn_id].frame_time;
        cfg_param->rc_attr.h265_cvbr.maxrate_percent = 200;
        cfg_param->rc_attr.h265_cvbr.IFrmMaxBits = 0;
        if (g_enc_chn_infos[grp_id][chn_id].breath_on)
        {
            cfg_param->rc_attr.h265_cvbr.IP_QPDelta = H265_BREATH_IP_QPDELTA;
            cfg_param->rc_attr.h265_cvbr.I_BitProp = H265_BREATH_I_BITPROP;
            cfg_param->rc_attr.h265_cvbr.P_BitProp = H265_BREATH_P_BITPROP;
        }
        else
        {
            cfg_param->rc_attr.h265_cvbr.IP_QPDelta = 3;
            cfg_param->rc_attr.h265_cvbr.I_BitProp = 5;
            cfg_param->rc_attr.h265_cvbr.P_BitProp = 1;
        }
        cfg_param->rc_attr.h265_cvbr.fluctuate_level = 0;
        cfg_param->rc_attr.h265_cvbr.stbitrate = g_enc_chn_infos[grp_id][chn_id].bps * 2;
        cfg_param->rc_attr.h265_cvbr.acceptable_qp = 32;
        break;

    case FH_RC_H265_QVBR:
        cfg_param->rc_attr.rc_type = FH_RC_H265_QVBR;
        cfg_param->rc_attr.h265_qvbr.bitrate = g_enc_chn_infos[grp_id][chn_id].bps;
        cfg_param->rc_attr.h265_qvbr.init_qp = 35;
        cfg_param->rc_attr.h265_qvbr.ImaxQP = 50;
        cfg_param->rc_attr.h265_qvbr.IminQP = 28;
        cfg_param->rc_attr.h265_qvbr.PmaxQP = 50;
        cfg_param->rc_attr.h265_qvbr.PminQP = 28;
        cfg_param->rc_attr.h265_qvbr.FrameRate.frame_count = g_enc_chn_infos[grp_id][chn_id].frame_count;
        cfg_param->rc_attr.h265_qvbr.FrameRate.frame_time = g_enc_chn_infos[grp_id][chn_id].frame_time;
        cfg_param->rc_attr.h265_qvbr.maxrate_percent = 200;
        cfg_param->rc_attr.h265_qvbr.IFrmMaxBits = 0;
        if (g_enc_chn_infos[grp_id][chn_id].breath_on)
        {
            cfg_param->rc_attr.h265_qvbr.IP_QPDelta = H265_BREATH_IP_QPDELTA;
            cfg_param->rc_attr.h265_qvbr.I_BitProp = H265_BREATH_I_BITPROP;
            cfg_param->rc_attr.h265_qvbr.P_BitProp = H265_BREATH_P_BITPROP;
        }
        else
        {
            cfg_param->rc_attr.h265_qvbr.IP_QPDelta = 3;
            cfg_param->rc_attr.h265_qvbr.I_BitProp = 5;
            cfg_param->rc_attr.h265_qvbr.P_BitProp = 1;
        }
        cfg_param->rc_attr.h265_qvbr.fluctuate_level = 0;
        cfg_param->rc_attr.h265_qvbr.minrate_percent = 30;
        cfg_param->rc_attr.h265_qvbr.psnr_ul = 3200;
        cfg_param->rc_attr.h265_qvbr.psnr_ll = 2900;
        break;

    case FH_RC_H265_MVBR:
        cfg_param->rc_attr.rc_type = FH_RC_H264_MVBR;
        cfg_param->rc_attr.h265_mvbr.init_qp = 35;
        cfg_param->rc_attr.h265_mvbr.stbitrate = g_enc_chn_infos[grp_id][chn_id].bps * 2;
        cfg_param->rc_attr.h265_mvbr.ltbitrate = g_enc_chn_infos[grp_id][chn_id].bps;
        cfg_param->rc_attr.h265_mvbr.maxrate_percent = 200;
        cfg_param->rc_attr.h265_mvbr.IminQP = 28;
        cfg_param->rc_attr.h265_mvbr.ImaxQP = 50;
        cfg_param->rc_attr.h265_mvbr.PminQP = 28;
        cfg_param->rc_attr.h265_mvbr.PmaxQP = 50;
        cfg_param->rc_attr.h265_mvbr.FrameRate.frame_count = g_enc_chn_infos[grp_id][chn_id].frame_count;
        cfg_param->rc_attr.h265_mvbr.FrameRate.frame_time = g_enc_chn_infos[grp_id][chn_id].frame_time;
        cfg_param->rc_attr.h265_mvbr.IFrmMaxBits = 0;
        if (g_enc_chn_infos[grp_id][chn_id].breath_on)
        {
            cfg_param->rc_attr.h265_mvbr.IP_QPDelta = H264_BREATH_IP_QPDELTA;
            cfg_param->rc_attr.h265_mvbr.I_BitProp = H264_BREATH_I_BITPROP;
            cfg_param->rc_attr.h265_mvbr.P_BitProp = H264_BREATH_P_BITPROP;
        }
        else
        {
            cfg_param->rc_attr.h265_mvbr.IP_QPDelta = 3;
            cfg_param->rc_attr.h265_mvbr.I_BitProp = 5;
            cfg_param->rc_attr.h265_mvbr.P_BitProp = 1;
        }
        cfg_param->rc_attr.h265_mvbr.fluctuate_level = 0;
        cfg_param->rc_attr.h265_mvbr.acceptable_qp = 32;
        cfg_param->rc_attr.h265_mvbr.stillrate_percent = 10;
        cfg_param->rc_attr.h265_mvbr.maxstillqp = 36;
        break;
    default:
        ret = -1;
        break;
    }
    SDK_FUNC_ERROR(model_name, ret);

    ret = FH_VENC_SetChnAttr(grp_id * MAX_VPU_CHN_NUM + chn_id, cfg_param);
    SDK_FUNC_ERROR(model_name, ret);

    return ret;
}

/* 设置H264编码通道 */
static FH_SINT32 set_h264(FH_SINT32 grp_id, FH_SINT32 chn_id)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;
    FH_VENC_CHN_CONFIG cfg_param;

    cfg_param.chn_attr.enc_type = FH_NORMAL_H264;
    cfg_param.chn_attr.h264_attr.profile = H264_PROFILE_MAIN;
    cfg_param.chn_attr.h264_attr.i_frame_intterval = 15;
    cfg_param.chn_attr.h264_attr.size.u32Width = g_enc_chn_infos[grp_id][chn_id].width;
    cfg_param.chn_attr.h264_attr.size.u32Height = g_enc_chn_infos[grp_id][chn_id].height;

    ret = set_264_rc(grp_id, chn_id, &cfg_param);

    return ret;
}

/* 设置H264智能编码通道 */
static FH_SINT32 set_s264(FH_SINT32 grp_id, FH_SINT32 chn_id)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;
    FH_VENC_CHN_CONFIG cfg_param;

    cfg_param.chn_attr.enc_type = FH_SMART_H264;
    cfg_param.chn_attr.s264_attr.profile = H264_PROFILE_MAIN;
    cfg_param.chn_attr.s264_attr.refresh_frame_intterval = 50;
    cfg_param.chn_attr.s264_attr.size.u32Width = g_enc_chn_infos[grp_id][chn_id].width;
    cfg_param.chn_attr.s264_attr.size.u32Height = g_enc_chn_infos[grp_id][chn_id].height;
    cfg_param.chn_attr.s264_attr.smart_en = FH_TRUE;
    cfg_param.chn_attr.s264_attr.texture_en = FH_TRUE;
    cfg_param.chn_attr.s264_attr.backgroudmodel_en = FH_TRUE;
    cfg_param.chn_attr.s264_attr.fresh_ltref_en = FH_FALSE;
    cfg_param.chn_attr.s264_attr.bgm_chn = grp_id;

    cfg_param.chn_attr.s264_attr.gop_th.GOP_TH_NUM = 4;
    cfg_param.chn_attr.s264_attr.gop_th.TH_VAL[0] = 8;
    cfg_param.chn_attr.s264_attr.gop_th.TH_VAL[1] = 15;
    cfg_param.chn_attr.s264_attr.gop_th.TH_VAL[2] = 25;
    cfg_param.chn_attr.s264_attr.gop_th.TH_VAL[3] = 35;
    cfg_param.chn_attr.s264_attr.gop_th.MIN_GOP[0] = 380;
    cfg_param.chn_attr.s264_attr.gop_th.MIN_GOP[1] = 330;
    cfg_param.chn_attr.s264_attr.gop_th.MIN_GOP[2] = 270;
    cfg_param.chn_attr.s264_attr.gop_th.MIN_GOP[3] = 220;
    cfg_param.chn_attr.s264_attr.gop_th.MIN_GOP[4] = 160;

    ret = set_264_rc(grp_id, chn_id, &cfg_param);

    return ret;
}

/* 设置H265编码通道 */
static FH_SINT32 set_h265(FH_SINT32 grp_id, FH_SINT32 chn_id)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;
    FH_VENC_CHN_CONFIG cfg_param;

    cfg_param.chn_attr.enc_type = FH_NORMAL_H265;
    cfg_param.chn_attr.h265_attr.profile = H265_PROFILE_MAIN;
    cfg_param.chn_attr.h265_attr.i_frame_intterval = 50;
    cfg_param.chn_attr.h265_attr.size.u32Width = g_enc_chn_infos[grp_id][chn_id].width;
    cfg_param.chn_attr.h265_attr.size.u32Height = g_enc_chn_infos[grp_id][chn_id].height;

    ret = set_265_rc(grp_id, chn_id, &cfg_param);

    return ret;
}

/* 设置H265智能编码通道 */
static FH_SINT32 set_s265(FH_SINT32 grp_id, FH_SINT32 chn_id)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;
    FH_VENC_CHN_CONFIG cfg_param;

    cfg_param.chn_attr.enc_type = FH_SMART_H265;
    cfg_param.chn_attr.s265_attr.profile = H265_PROFILE_MAIN;
    cfg_param.chn_attr.s265_attr.refresh_frame_intterval = 50;
    cfg_param.chn_attr.s265_attr.size.u32Width = g_enc_chn_infos[grp_id][chn_id].width;
    cfg_param.chn_attr.s265_attr.size.u32Height = g_enc_chn_infos[grp_id][chn_id].height;
    cfg_param.chn_attr.s265_attr.smart_en = FH_TRUE;
    cfg_param.chn_attr.s265_attr.texture_en = FH_TRUE;
    cfg_param.chn_attr.s265_attr.backgroudmodel_en = FH_TRUE;
    cfg_param.chn_attr.s265_attr.fresh_ltref_en = FH_FALSE;
    cfg_param.chn_attr.s265_attr.bgm_chn = grp_id;

    cfg_param.chn_attr.s265_attr.gop_th.GOP_TH_NUM = 4;
    cfg_param.chn_attr.s265_attr.gop_th.TH_VAL[0] = 8;
    cfg_param.chn_attr.s265_attr.gop_th.TH_VAL[1] = 15;
    cfg_param.chn_attr.s265_attr.gop_th.TH_VAL[2] = 25;
    cfg_param.chn_attr.s265_attr.gop_th.TH_VAL[3] = 35;
    cfg_param.chn_attr.s265_attr.gop_th.MIN_GOP[0] = 380;
    cfg_param.chn_attr.s265_attr.gop_th.MIN_GOP[1] = 330;
    cfg_param.chn_attr.s265_attr.gop_th.MIN_GOP[2] = 270;
    cfg_param.chn_attr.s265_attr.gop_th.MIN_GOP[3] = 220;
    cfg_param.chn_attr.s265_attr.gop_th.MIN_GOP[4] = 160;

    ret = set_265_rc(grp_id, chn_id, &cfg_param);

    return ret;
}

static FH_SINT32 set_breath(FH_SINT32 grp_id, FH_SINT32 chn_id)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;
    // 默认常开
    return ret;
}

ENC_INFO *get_enc_config(FH_SINT32 grp_id, FH_SINT32 chn_id)
{
    return &g_enc_chn_infos[grp_id][chn_id];
}

FH_SINT32 enc_mod_set(FH_VOID)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;
    FH_VENC_MOD_PARAM h264_param;
    FH_VENC_MOD_PARAM jpeg_param;
    FH_VENC_MOD_PARAM mjpeg_param;
    FH_SINT32 jpeg_enable = 0;
    FH_SINT32 mjpeg_enable = 0;
    ENC_INFO *mjpeg_info;
    FH_SINT32 enc_enable = 0;
    ENC_INFO *enc_info;

#ifdef FH_APP_CAPTURE_JPEG
    jpeg_enable = 1;
#else
    jpeg_enable = 0;
#endif

    for (FH_SINT32 grp_id = 0; grp_id < MAX_GRP_NUM; grp_id++)
    {
        for (FH_SINT32 chn_id = 0; chn_id < MAX_VPU_CHN_NUM; chn_id++)
        {
            mjpeg_info = get_mjpeg_config(grp_id, chn_id);
            enc_info = get_enc_config(grp_id, chn_id);

            if (enc_info->enable)
            {
                enc_enable = 1;
            }

            if (mjpeg_info->enable)
            {
                mjpeg_enable = 1;
            }
        }
    }

    if (jpeg_enable)
    {
        jpeg_param.type = FH_STREAM_JPEG;
        ret = FH_VENC_GetModParam(&jpeg_param);
        SDK_FUNC_ERROR(model_name, ret);

        jpeg_param.stm_size = JPEG_COMMON_BUFFER_SIZE;
        jpeg_param.max_stm_num = 128;
        jpeg_param.stm_cache = 0;

        ret = FH_VENC_SetModParam(&jpeg_param);
        SDK_FUNC_ERROR(model_name, ret);
    }

    if (mjpeg_enable)
    {
        mjpeg_param.type = FH_STREAM_MJPEG;
        ret = FH_VENC_GetModParam(&mjpeg_param);
        SDK_FUNC_ERROR(model_name, ret);

        mjpeg_param.stm_size = MJPEG_COMMON_BUFFER_SIZE;
        mjpeg_param.max_stm_num = 128;
        mjpeg_param.stm_cache = 0;

        ret = FH_VENC_SetModParam(&mjpeg_param);
        SDK_FUNC_ERROR(model_name, ret);
    }

    if (enc_enable)
    {
        h264_param.type = FH_STREAM_H264;
        ret = FH_VENC_GetModParam(&h264_param);
        SDK_FUNC_ERROR(model_name, ret);

        h264_param.stm_size = H264_COMMON_BUFFER_SIZE;
        h264_param.max_stm_num = 128;
        h264_param.stm_cache = 0;

        ret = FH_VENC_SetModParam(&h264_param);
        SDK_FUNC_ERROR(model_name, ret);
    }

    return ret;
}

static FH_SINT32 enc_set_chn_buffer(FH_SINT32 chn_id)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;
    FH_VENC_CHN_PARAM pstchnparam;

    ret = FH_VENC_GetChnParam(chn_id, &pstchnparam);
    SDK_FUNC_ERROR(model_name, ret);

#ifdef UVC_ENABLE // 独立缓冲
    pstchnparam.stm_mode = FH_STM_CHN_BUF;
    pstchnparam.stm_size = H264_SINGLE_BUFFER; // 需要根据幅面大小进行设置，默认2MByte
    pstchnparam.max_stm_num = 32;
#else // 公共缓冲
    pstchnparam.stm_mode = FH_STM_PUB_BUF;
#endif

    ret = FH_VENC_SetChnParam(chn_id, &pstchnparam);
    SDK_FUNC_ERROR(model_name, ret);

    return ret;
}

FH_SINT32 enc_create_chn(FH_SINT32 grp_id, FH_SINT32 chn_id)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;
    FH_VENC_CHN_CAP cfg_vencmem;

    ret = enc_set_chn_buffer(grp_id * MAX_VPU_CHN_NUM + chn_id);
    SDK_FUNC_ERROR(model_name, ret);

#if defined CH0_LPBUF_ENABLE_G0 || defined CH0_LPBUF_ENABLE_G1
    if (chn_id == 0)
    {
        cfg_vencmem.work_mode = FH_HARD_MODE;
    }
    else
    {
        cfg_vencmem.work_mode = FH_NORMAL_MODE;
    }
#else
    cfg_vencmem.work_mode = FH_NORMAL_MODE;
#endif
    cfg_vencmem.support_type = g_enc_chn_infos[grp_id][chn_id].enc_type;
    cfg_vencmem.max_size.u32Width = g_enc_chn_infos[grp_id][chn_id].max_width;
    cfg_vencmem.max_size.u32Height = g_enc_chn_infos[grp_id][chn_id].max_height;

    ret = FH_VENC_CreateChn(grp_id * MAX_VPU_CHN_NUM + chn_id, &cfg_vencmem);
    SDK_FUNC_ERROR(model_name, ret);

    g_enc_chn_infos[grp_id][chn_id].channel = grp_id * MAX_VPU_CHN_NUM + chn_id;

    return ret;
}

FH_SINT32 enc_set_chn_attr(FH_SINT32 grp_id, FH_SINT32 chn_id)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;
    FH_UINT32 enc_type = g_enc_chn_infos[grp_id][chn_id].enc_type;

    switch (enc_type)
    {
    case FH_NORMAL_H264:
        ret = set_h264(grp_id, chn_id);
        break;
    case FH_SMART_H264:
        ret = set_s264(grp_id, chn_id);
        break;
    case FH_NORMAL_H265:
        ret = set_h265(grp_id, chn_id);
        break;
    case FH_SMART_H265:
        ret = set_s265(grp_id, chn_id);
        break;
    default:
        ret = 0;
        break;
    }

    SDK_FUNC_ERROR(model_name, ret);

    if ((enc_type != FH_JPEG) && (enc_type != FH_MJPEG))
    {
        ret = set_breath(grp_id, chn_id);
    }

    return ret;
}

FH_SINT32 set_enc_rotate(FH_SINT32 grp_id, FH_SINT32 chn_id, FH_SINT32 rotate)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;
    FH_ROTATE_OPS pstVencrotateinfo = FH_RO_OPS_0;

    if (rotate != 270 && rotate != 180 && rotate != 90 && rotate != 0)
    {
        SDK_ERR_PRT(model_name, "parama rotate need [0,90,180,270]!\n");
        return FH_SDK_FAILED;
    }

    switch (rotate)
    {
    case 0:
        pstVencrotateinfo = FH_RO_OPS_0;
        break;
    case 90:
        pstVencrotateinfo = FH_RO_OPS_90;
        break;
    case 180:
        pstVencrotateinfo = FH_RO_OPS_180;
        break;
    case 270:
        pstVencrotateinfo = FH_RO_OPS_270;
        break;
    default:
        break;
    }
    SDK_PRT(model_name, "enc chn = %d,pstVencrotateinfo=%d\n", g_enc_chn_infos[grp_id][chn_id].channel, pstVencrotateinfo);
    ret = FH_VENC_SetRotate(g_enc_chn_infos[grp_id][chn_id].channel, pstVencrotateinfo);
    SDK_FUNC_ERROR(model_name, ret);

    return ret;
}

FH_SINT32 enc_start(FH_SINT32 grp_id, FH_SINT32 chn_id)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;

    ret = FH_VENC_StartRecvPic(g_enc_chn_infos[grp_id][chn_id].channel);
    SDK_FUNC_ERROR(model_name, ret);

    g_enc_chn_infos[grp_id][chn_id].run_status = 1;

    return ret;
}

FH_SINT32 enc_stop(FH_SINT32 grp_id, FH_SINT32 chn_id)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;

    if (g_enc_chn_infos[grp_id][chn_id].run_status)
    {
        ret = FH_VENC_StopRecvPic(g_enc_chn_infos[grp_id][chn_id].channel);
        SDK_FUNC_ERROR(model_name, ret);

        g_enc_chn_infos[grp_id][chn_id].run_status = 0;
    }

    return ret;
}

FH_SINT32 enc_destroy(FH_SINT32 grp_id, FH_SINT32 chn_id)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;

    ret = FH_VENC_DestroyChn(g_enc_chn_infos[grp_id][chn_id].channel);
    SDK_FUNC_ERROR(model_name, ret);

    return ret;
}

FH_SINT32 enc_sendframe(FH_SINT32 chn_id, FH_ENC_FRAME *frame, FH_UINT32 timeout_ms)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;

    ret = FH_VENC_Submit_ENC_Ex(chn_id, frame, VPU_VOMODE_SCAN);
    SDK_FUNC_ERROR(model_name, ret);

    return ret;
}

// TODO new add
FH_SINT32 enc_get_chn_status(FH_SINT32 grp_id, FH_SINT32 chn_id, FH_CHN_STATUS *chnstat)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;

    ret = FH_VENC_GetChnStatus(grp_id * MAX_VPU_CHN_NUM + chn_id, chnstat);
    SDK_FUNC_ERROR(model_name, ret);

    return ret;
}

FH_SINT32 enc_get_common_stream(FH_UINT32 request_type, FH_VENC_STREAM *stream)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;

    WR_PROC_DEV(TRACE_PROC, "timing_GetStream_START");

    ret = FH_VENC_GetStream_Timeout(request_type, stream, 10 * 1000);
    SDK_FUNC_ERROR(model_name, ret);

    WR_PROC_DEV(TRACE_PROC, "timing_GetStream_END");

    return ret;
}

FH_SINT32 enc_get_chn_stream(FH_UINT32 enc_chn, FH_VENC_STREAM *stream)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;

    WR_PROC_DEV(TRACE_PROC, "timing_GetChnStream_START");

    ret = FH_VENC_GetChnStream_Timeout(enc_chn, stream, 10 * 1000);
    SDK_FUNC_ERROR(model_name, ret);

    WR_PROC_DEV(TRACE_PROC, "timing_GetChnStream_END");

    return ret;
}

FH_SINT32 enc_release_stream(FH_VENC_STREAM *stream)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;

    WR_PROC_DEV(TRACE_PROC, "timing_ReleaseStream_START");

    ret = FH_VENC_ReleaseStream(stream);
    SDK_FUNC_ERROR(model_name, ret);

    WR_PROC_DEV(TRACE_PROC, "timing_ReleaseStream_END");

    return ret;
}

// 下面全部放到common里面
// FH_SINT32 IPC_GetStream(FH_VOID)
FH_SINT32 getAllEncStreamToDmc(FH_VOID)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;
    FH_SINT32 i;
    FH_SINT32 end_flag;
    FH_SINT32 subtype;
    FH_VENC_STREAM stream;

    memset(&stream, 0, sizeof(stream));
    /*阻塞模式下,获取一帧H264,H265,MJPEG数据*/
    ret = enc_get_common_stream(FH_STREAM_ALL & (~(FH_STREAM_JPEG)), &stream);
    SDK_FUNC_ERROR(model_name, ret);

    /*获取到一帧H264数据,按照下面的方式处理*/
    if (stream.stmtype == FH_STREAM_H264)
    {
        subtype = stream.h264_stream.frame_type == FH_FRAME_I ? DMC_MEDIA_SUBTYPE_IFRAME : DMC_MEDIA_SUBTYPE_PFRAME;
        for (i = 0; i < stream.h264_stream.nalu_cnt; i++)
        {
            end_flag = (i == (stream.h264_stream.nalu_cnt - 1)) ? 1 : 0;
            dmc_input(stream.chan,
                      DMC_MEDIA_TYPE_H264,
                      subtype,
                      stream.h264_stream.time_stamp,
                      stream.h264_stream.nalu[i].start,
                      stream.h264_stream.nalu[i].length,
                      end_flag);
        }
    }

    /*获取到一帧H265数据,按照下面的方式处理*/
    else if (stream.stmtype == FH_STREAM_H265)
    {
        subtype = stream.h265_stream.frame_type == FH_FRAME_I ? DMC_MEDIA_SUBTYPE_IFRAME : DMC_MEDIA_SUBTYPE_PFRAME;
        for (i = 0; i < stream.h265_stream.nalu_cnt; i++)
        {
            end_flag = (i == (stream.h265_stream.nalu_cnt - 1)) ? 1 : 0;
            dmc_input(stream.chan,
                      DMC_MEDIA_TYPE_H265,
                      subtype,
                      stream.h265_stream.time_stamp,
                      stream.h265_stream.nalu[i].start,
                      stream.h265_stream.nalu[i].length,
                      end_flag);
        }
    }

    /*获取到一帧MJPEG数据,按照下面的方式处理*/
    else if (stream.stmtype == FH_STREAM_MJPEG)
    {
        dmc_input(stream.chan,
                  DMC_MEDIA_TYPE_MJPEG,
                  0,
                  0,
                  stream.mjpeg_stream.start,
                  stream.mjpeg_stream.length,
                  1);
    }

    ret = enc_release_stream(&stream);
    SDK_FUNC_ERROR(model_name, ret);

    return ret;
}

FH_SINT32 enc_chn_relase(FH_SINT32 grp_id, FH_SINT32 chn_id)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;

    if (!g_enc_chn_infos[grp_id][chn_id].enable)
    {
        SDK_ERR_PRT(model_name, "enc_chn_relase Failed, VPU[%d] CHN[%d] ENC not enable!\n", grp_id, chn_id);
        return FH_SDK_FAILED;
    }

    ret = enc_stop(grp_id, chn_id);
    SDK_FUNC_ERROR(model_name, ret);

    ret = enc_destroy(grp_id, chn_id);
    SDK_FUNC_ERROR(model_name, ret);

    return ret;
}

FH_SINT32 changeEncAttr(FH_SINT32 grp_id, FH_SINT32 chn_id, FH_VENC_TYPE encType, FH_VENC_RC_MODE rcType, FH_SINT32 width, FH_SINT32 height)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;

    if (!g_enc_chn_infos[grp_id][chn_id].enable)
    {
        SDK_ERR_PRT(model_name, "changeEncAttr Failed, VPU[%d] CHN[%d] ENC not enable!\n", grp_id, chn_id);
        return FH_SDK_FAILED;
    }

    if (encType != 0)
    {
        g_enc_chn_infos[grp_id][chn_id].enc_type = encType;
        g_enc_chn_infos[grp_id][chn_id].rc_type = rcType;
    }

    g_enc_chn_infos[grp_id][chn_id].max_width = width;
    g_enc_chn_infos[grp_id][chn_id].max_height = height;
    g_enc_chn_infos[grp_id][chn_id].width = width;
    g_enc_chn_infos[grp_id][chn_id].height = height;

    ret = enc_create_chn(grp_id, chn_id);
    SDK_FUNC_ERROR(model_name, ret);

    ret = enc_set_chn_attr(grp_id, chn_id);
    SDK_FUNC_ERROR(model_name, ret);

    ret = enc_start(grp_id, chn_id);
    SDK_FUNC_ERROR(model_name, ret);

    return ret;
}

FH_SINT32 getENCStreamBsp(FH_SINT32 grp_id, FH_SINT32 chn_id, FH_UINT32 *bsp) // 移动到common里
{
    FH_SINT32 ret = FH_SDK_SUCCESS;
    FH_CHN_STATUS chnstat;
    ENC_INFO *enc_info;

    memset(&chnstat, 0, sizeof(chnstat));
    enc_info = get_enc_config(grp_id, chn_id);
    if (!enc_info->enable)
    {
        *bsp = 0;
        return FH_SDK_SUCCESS;
    }
    else
    {
        ret = enc_get_chn_status(grp_id, chn_id, &chnstat);
        SDK_FUNC_ERROR(model_name, ret);

        *bsp = chnstat.bps;
    }

    return ret;
}
