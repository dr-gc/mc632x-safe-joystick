#include "nna_config.h"

static char *model_name = MODEL_TAG_NNA_CONFIG;

static pthread_mutex_t g_nna_lock = PTHREAD_MUTEX_INITIALIZER;
static FH_MODEL_INFO *g_modelHandle[MODEL_MAX] = {NULL};

static FH_SINT32 chooseNNType(NN_TYPE nn_type, FH_UINT32 *type)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;

    *type = 0;

    switch (nn_type)
    {
    case PERSON_DET:
        *type = FN_DET_TYPE_PERSON;
        break;
    case PERSON_VEHICLE_DET:
        *type = FN_DET_TYPE_C2;
        break;
    case FACE_DET:
        *type = FN_DET_TYPE_FACE;
        break;
#if defined(FH_FACE_SNAP_ENABLE) || defined(FH_FACE_REC_ENABLE)
    case FACE_KPT_DET:
        *type = FN_DET_TYPE_FACE_FULL;
        break;
#endif
#if defined(FH_FACE_REC_ENABLE)
    case FACE_REC:
        *type = FN_DET_TYPE_FACEREC;
        break;
#endif
#if defined(FH_GESTURE_REC_ENABLE)
    case GES_DET:
        *type = FN_GESTURE_DET;
        break;
    case GES_REC:
        *type = FN_GESTUREREC;
        break;
#endif
#if defined(FH_LP_REC_ENABLE)
    case LP_DET:
        *type = FN_DET_TYPE_LPD_FULL;
        break;
    case LP_REC:
        *type = FN_DET_TYPE_LPREC;
        break;
#endif
    default:
        SDK_ERR_PRT(model_name, "nn_type[%d] not found!\n", nn_type);
        break;
    }

    if (*type == 0)
    {
        ret = FH_SDK_FAILED;
    }

    return ret;
}

static FH_SINT32 chooseFormat(NN_IMAGE_TYPE imageType, FH_UINT32 *data_format)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;

    *data_format = 0;

    switch (imageType)
    {
    case IMAGE_TYPE_RGB888:
        *data_format = VPU_VOMODE_ARGB888;
        break;
    default:
        SDK_ERR_PRT(model_name, "NNA data_format[%d] not found!\n", *data_format);
        break;
    }

    if (*data_format == 0)
    {
        ret = FH_SDK_FAILED;
    }

    return ret;
}

#if defined(FH_FACE_REC_ENABLE) || defined(FH_LP_REC_ENABLE)
FH_SINT32 nna_algo_format(NN_IMAGE_TYPE nn_image_type, FHA_IMAGE_TYPE_e *imageType, FH_FLOAT *chn, FH_UINT32 *stride_coef)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;

    *imageType = 0;
    *chn = 0;
    *stride_coef = 1;

    switch (nn_image_type)
    {
    case IMAGE_TYPE_RGB888:
        *imageType = FH_IMAGE_FORMAT_RGBA;
        *chn = 4.0f;
        *stride_coef = 4;
        break;
    case IMAGE_TYPE_RRGGBB:
        *imageType = FH_IMAGE_FORMAT_RGB_PLANE;
        *chn = 3.0f;
        break;
    case IMAGE_TYPE_YUV420:
        *imageType = FH_IMAGE_FORMAT_I420;
        *chn = 1.5f;
        break;
    default:
        SDK_ERR_PRT(model_name, "NNA imageType[%d] not found!\n", *imageType);
        break;
    }

    if (*imageType == 0)
    {
        ret = FH_SDK_FAILED;
    }

    return ret;
}
#endif

static FH_SINT32 nna_post_create(NN_HANDLE_t *handle_t, FH_MEM_INFO pp_conf)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;
    NnPostInitParam post_param;

    SDK_CHECK_NULL_PTR(model_name, handle_t);

    memset(&post_param, 0, sizeof(post_param));
    post_param.pp_conf_in.base = pp_conf.base;
    post_param.pp_conf_in.vbase = pp_conf.vbase;
    post_param.pp_conf_in.size = pp_conf.size;
    post_param.config_th = 0.8;
    post_param.rotate = FN_ROT_0; //set rotate FN_ROT_90

    switch (handle_t->model_info->type)
    {
#if defined(FH_NN_DETECT_ENABLE)
    case PERSON_DET:
    case PERSON_VEHICLE_DET:
    case FACE_DET:
        ret = FH_NNPost_PrimaryDet_Create(&(handle_t->model_info->postHdl), &post_param);
        SDK_FUNC_ERROR_GOTO(model_name, ret);
        SDK_PRT(model_name, "NNA[%d] Det Post Create success!\n", handle_t->id);
        break;
#endif
#if defined(FH_FACE_SNAP_ENABLE) || defined(FH_FACE_REC_ENABLE)
    case FACE_KPT_DET:
        ret = FH_NNPost_FACE_DET_FULL_Create(&(handle_t->model_info->postHdl), &post_param);
        SDK_FUNC_ERROR_GOTO(model_name, ret);
        SDK_PRT(model_name, "NNA[%d] Face Snap Post Create success!\n", handle_t->id);
        break;
#endif
#if defined(FH_FACE_REC_ENABLE)
    case FACE_REC:
        ret = FH_NNPost_FACEREC_Create(&(handle_t->model_info->postHdl), &post_param);
        SDK_FUNC_ERROR_GOTO(model_name, ret);
        SDK_PRT(model_name, "NNA[%d] Face Rec Post Create success!\n", handle_t->id);
        break;
#endif
#if defined(FH_GESTURE_REC_ENABLE)
    case GES_DET:
        ret = FH_NNPost_GESTURE_DET_Create(&(handle_t->model_info->postHdl), &post_param);
        SDK_FUNC_ERROR_GOTO(model_name, ret);
        SDK_PRT(model_name, "NNA[%d] Gesture Det Post Create success!\n", handle_t->id);
        break;
    case GES_REC:
        ret = FH_NNPost_GESTUREREC_Create(&(handle_t->model_info->postHdl), &post_param);
        SDK_FUNC_ERROR_GOTO(model_name, ret);
        SDK_PRT(model_name, "NNA[%d] Gesture Rec Post Create success!\n", handle_t->id);
        break;
#endif
#if defined(FH_LP_REC_ENABLE)
    case LP_DET:
        ret = FH_NNPost_LP_DET_FULL_Create(&(handle_t->model_info->postHdl), &post_param);
        SDK_FUNC_ERROR_GOTO(model_name, ret);
        SDK_PRT(model_name, "NNA[%d] LP Det Post Create success!\n", handle_t->id);
        break;
    case LP_REC:
        ret = FH_NNPost_LPREC_Create(&(handle_t->model_info->postHdl), &post_param);
        SDK_FUNC_ERROR_GOTO(model_name, ret);
        SDK_PRT(model_name, "NNA[%d] LP Rec Post Create success!\n", handle_t->id);
        break;
#endif
    default:
        // not need Post
        break;
    }

    return ret;

Exit:
    handle_t->model_info->postHdl = NULL;

    return FH_SDK_FAILED;
}

static FH_SINT32 nna_post_release(NN_HANDLE_t *handle_t)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;

    SDK_CHECK_NULL_PTR(model_name, handle_t);

    switch (handle_t->model_info->type)
    {
#if defined(FH_NN_DETECT_ENABLE)
    case PERSON_DET:
    case PERSON_VEHICLE_DET:
    case FACE_DET:
        ret = FH_NNPost_PrimaryDet_Destroy(handle_t->model_info->postHdl);
        SDK_FUNC_ERROR_PRT(model_name, ret);
        break;
#endif
#if defined(FH_FACE_SNAP_ENABLE) || defined(FH_FACE_REC_ENABLE)
    case FACE_KPT_DET:
        ret = FH_NNPost_FACE_DET_FULL_Destroy(handle_t->model_info->postHdl);
        SDK_FUNC_ERROR_PRT(model_name, ret);
        break;
#endif
#if defined(FH_FACE_REC_ENABLE)
    case FACE_REC:
        ret = FH_NNPost_FACEREC_Destroy(handle_t->model_info->postHdl);
        SDK_FUNC_ERROR_PRT(model_name, ret);
        break;
#endif
#if defined(FH_GESTURE_REC_ENABLE)
    case GES_DET:
        ret = FH_NNPost_GESTURE_DET_Destroy(handle_t->model_info->postHdl);
        SDK_FUNC_ERROR_PRT(model_name, ret);
        break;
    case GES_REC:
        ret = FH_NNPost_GESTUREREC_Destroy(handle_t->model_info->postHdl);
        SDK_FUNC_ERROR_PRT(model_name, ret);
        break;
#endif
#if defined(FH_LP_REC_ENABLE)
    case LP_DET:
        ret = FH_NNPost_LP_DET_FULL_Destroy(handle_t->model_info->postHdl);
        SDK_FUNC_ERROR_PRT(model_name, ret);
        break;
    case LP_REC:
        ret = FH_NNPost_LP_DET_FULL_Destroy(handle_t->model_info->postHdl);
        SDK_FUNC_ERROR_PRT(model_name, ret);
        break;
#endif
    default:
        // not need Post
        break;
    }

    handle_t->model_info->postHdl = NULL;

    return ret;

Exit:
    return FH_SDK_FAILED;
}

FH_SINT32 nna_init(FH_SINT32 nn_id, NN_HANDLE *nn_handle, NN_TYPE nn_type, FH_CHAR *model_path, FH_SINT32 model_width, FH_SINT32 model_height)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;
    NN_HANDLE_t *handle_t = NULL;
    FH_NN_MODEL_INIT_PARAM_T model_param;
    FH_NN_TASK_INIT_PARAM_T task_param;
    FH_MEM_INFO pp_conf;
    FH_SINT32 model_from_mem = 0;   /* 1=模型来自固件静态区(mem:前缀)，不可 free */

    SDK_CHECK_NULL_PTR(model_name, model_path);

    if (*nn_handle != NULL)
    {
        SDK_FREE_PTR(*nn_handle);
        SDK_PRT(model_name, "NNA[%d] nn_handle already create!\n", nn_id);
        return FH_SDK_FAILED;
    }

    pthread_mutex_lock(&g_nna_lock);

    handle_t = (NN_HANDLE_t *)malloc(sizeof(NN_HANDLE_t));
    memset(handle_t, 0, sizeof(NN_HANDLE_t));

    handle_t->id = nn_id;
    memset(&model_param, 0, sizeof(model_param));
    if (g_modelHandle[nn_type] == NULL)
    {
        SDK_PRT(model_name, "NNA[%d] g_model[%d] NULL Create g_model!\n", nn_id, nn_type);
        g_modelHandle[nn_type] = (FH_MODEL_INFO *)malloc(sizeof(FH_MODEL_INFO));
        memset(g_modelHandle[nn_type], 0, sizeof(FH_MODEL_INFO));
        handle_t->model_info = g_modelHandle[nn_type];

        handle_t->model_info->mdl_path = model_path;
        handle_t->model_info->mdl_width = model_width;
        handle_t->model_info->mdl_height = model_height;
        handle_t->model_info->type = nn_type;

        model_param.src_w_in = model_width;
        model_param.src_h_in = model_height;
        model_param.src_c_in = 4;
        /* 支持 "mem:0xXXXXXXXX" 前缀 —— 模型已由 u-boot tftp 下载到该 RAM 地址，
         * 直接使用，不读文件系统（板上 jffs2 无法传大文件时用此路径）*/
        if (model_path && strncmp(model_path, "mem:", 4) == 0)
        {
            unsigned long addr = strtoul(model_path + 4, NULL, 0);
            model_param.nbg_data_mem.vbase = (FH_VOID *)addr;
            model_from_mem = 1;
            /* u-boot 也会在同地址前写入长度，这里打印便于确认 */
            SDK_PRT(model_name, "NNA[%d] load model from MEM 0x%lx (skip fs)\n", nn_id, addr);
        }
        else
        {
            model_param.nbg_data_mem.vbase = readFile(model_path); // load model
        }
        SDK_CHECK_NULL_PTR(model_name, model_param.nbg_data_mem.vbase);

        ret = chooseNNType(nn_type, &model_param.type);
        SDK_FUNC_ERROR_GOTO(model_name, ret);

        ret = FH_NNA_Model_Init(&handle_t->model_info->mdlHdl, &model_param, NULL);
        SDK_FUNC_ERROR_GOTO(model_name, ret);
        SDK_PRT(model_name, "NNA[%d] FH_NNA_Model_Init success! g_model[%d] = %p, mdlHdl = %p\n", nn_id, nn_type, g_modelHandle[nn_type], handle_t->model_info->mdlHdl);

        ret = FH_NNA_GetPpconfig(handle_t->model_info->mdlHdl, &pp_conf);
        SDK_FUNC_ERROR_GOTO(model_name, ret);

        ret = nna_post_create(handle_t, pp_conf);
        SDK_FUNC_ERROR_GOTO(model_name, ret);
    }
    else
    {
        handle_t->model_info = g_modelHandle[nn_type];
        SDK_PRT(model_name, "NNA[%d] g_model not NULL g_model[%d] = %p!, mdlHdl = %p\n", nn_id, nn_type, g_modelHandle[nn_type], handle_t->model_info->mdlHdl);
    }
    SDK_CHECK_NULL_PTR(model_name, handle_t->model_info->mdlHdl);
    pthread_mutex_unlock(&g_nna_lock);

    memset(&task_param, 0, sizeof(task_param));

    switch (nn_type)
    {
    case FACE_KPT_DET:
        task_param.rotate = FN_ROT_0; // set rotate FN_ROT_90
        break;
    default:
        task_param.rotate = FN_ROT_0;
        break;
    }

    ret = FH_NNA_Task_Init(&(handle_t->tskHdl), handle_t->model_info->mdlHdl, &task_param, NULL);
    SDK_FUNC_ERROR_GOTO(model_name, ret);
    handle_t->model_info->task_count++;

    SDK_PRT(model_name, "NNA[%d] Task Create success!\n", nn_id);

    /* ★ 只有 readFile() 分配的内存才能 free，mem: 前缀指向固件静态数据，不能释放 */
    if (!model_from_mem)
    {
        SDK_FREE_PTR(model_param.nbg_data_mem.vbase);
    }
    else
    {
        SDK_PRT(model_name, "NNA[%d] skip free (static model in firmware)\n", nn_id);
    }

    *nn_handle = handle_t;

    return ret;

Exit:
    if (handle_t->tskHdl)
    {
        ret = FH_NNA_ReleaseTask(handle_t->tskHdl);
        SDK_FUNC_ERROR_PRT(model_name, ret);
        if (!ret)
        {
            handle_t->tskHdl = NULL;
            pthread_mutex_lock(&g_nna_lock);
            handle_t->model_info->task_count--;
            pthread_mutex_unlock(&g_nna_lock);
        }
    }

    if (handle_t->model_info->task_count <= 0)
    {
        if (handle_t->model_info->postHdl)
        {
            ret = nna_post_release(handle_t);
            SDK_FUNC_ERROR_PRT(model_name, ret);
        }

        if (handle_t->model_info->mdlHdl)
        {
            ret = FH_NNA_ReleaseModel(handle_t->model_info->mdlHdl);
            SDK_FUNC_ERROR_PRT(model_name, ret);
            handle_t->model_info->mdlHdl = NULL;
        }

        if (!ret)
        {
            pthread_mutex_lock(&g_nna_lock);
            SDK_FREE_PTR(g_modelHandle[handle_t->model_info->type]);
            g_modelHandle[handle_t->model_info->type] = NULL;
            pthread_mutex_unlock(&g_nna_lock);
        }
    }

    if (model_param.nbg_data_mem.vbase && !model_from_mem)
    {
        SDK_FREE_PTR(model_param.nbg_data_mem.vbase);
    }
    SDK_FREE_PTR(handle_t);
    ret = FH_SDK_FAILED;

    return ret;
}

FH_SINT32 nna_release(FH_SINT32 nn_id, NN_HANDLE *nn_handle)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;
    NN_HANDLE_t *handle_t = NULL;

    SDK_CHECK_NULL_PTR(model_name, nn_handle);

    handle_t = (NN_HANDLE_t *)*nn_handle;
    SDK_CHECK_NULL_PTR(model_name, handle_t);

    if (handle_t->tskHdl)
    {
        ret = FH_NNA_ReleaseTask(handle_t->tskHdl);
        SDK_FUNC_ERROR_PRT(model_name, ret);
        if (!ret)
        {
            handle_t->tskHdl = NULL;
            pthread_mutex_lock(&g_nna_lock);
            handle_t->model_info->task_count--;
            pthread_mutex_unlock(&g_nna_lock);
        }
    }

    if (handle_t->model_info->task_count <= 0)
    {
        if (handle_t->model_info->postHdl)
        {
            ret = nna_post_release(handle_t);
            SDK_FUNC_ERROR_PRT(model_name, ret);
        }

        if (handle_t->model_info->mdlHdl)
        {
            ret = FH_NNA_ReleaseModel(handle_t->model_info->mdlHdl);
            SDK_FUNC_ERROR_PRT(model_name, ret);
            handle_t->model_info->mdlHdl = NULL;
        }

        if (!ret)
        {
            pthread_mutex_lock(&g_nna_lock);
            SDK_FREE_PTR(g_modelHandle[handle_t->model_info->type]);
            g_modelHandle[handle_t->model_info->type] = NULL;
            pthread_mutex_unlock(&g_nna_lock);
        }
    }

    SDK_FREE_PTR(handle_t);

    return ret;
Exit:
    return FH_SDK_FAILED;
}

FH_SINT32 nna_get_obj_det_result(NN_HANDLE nn_handle, NN_INPUT_DATA nn_input_data, NN_RESULT *nn_result)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;
    NN_HANDLE_t *handle_t = NULL;
    FH_VIDEO_FRAME nn_src;
    FH_NN_OUT_INFO hw_out_info;
    FH_DETECTION_T det_out;
    FH_NN_POST_INPUT_INFO post_input_info;
    FH_UINT32 data_format;

    SDK_CHECK_NULL_PTR(model_name, nn_handle);

    handle_t = (NN_HANDLE_t *)nn_handle;
    SDK_CHECK_NULL_PTR(model_name, handle_t);
    SDK_CHECK_NULL_PTR(model_name, nn_result);

    nn_src.size.u32Width = nn_input_data.width;
    nn_src.size.u32Height = nn_input_data.height;

    ret = chooseFormat(nn_input_data.imageType, &data_format);
    SDK_FUNC_ERROR(model_name, ret);

    nn_src.data_format = data_format;
    nn_src.frm_argb8888.data.base = nn_input_data.data.base;
    nn_src.frm_argb8888.data.vbase = nn_input_data.data.vbase;
    nn_src.frm_argb8888.data.size = nn_input_data.data.size;
    nn_src.time_stamp = nn_input_data.timestamp;
    nn_src.frame_id = nn_input_data.frame_id;
    nn_src.pool_id = nn_input_data.pool_id;

    ret = FH_NNA_DetectProcess(handle_t->tskHdl, &nn_src, &hw_out_info);
    SDK_FUNC_ERROR(model_name, ret);

    memset(&post_input_info, 0, sizeof(post_input_info));
    post_input_info.hw_output_mem.base = hw_out_info.hw_output_mem.base;
    post_input_info.hw_output_mem.vbase = hw_out_info.hw_output_mem.vbase;
    post_input_info.hw_output_mem.size = hw_out_info.hw_output_mem.size;
    post_input_info.current_rotate = hw_out_info.current_rotate;
    post_input_info.current_stamp = hw_out_info.current_stamp;
    post_input_info.frame_id = hw_out_info.frame_id;
    ret = FH_NNPost_PrimaryDet_Process(handle_t->model_info->postHdl, &post_input_info, &det_out);
    SDK_FUNC_ERROR_GOTO(model_name, ret);

    nn_result->result_det.nn_result_num = det_out.boxNum;

#if NN_DEBUG
    SDK_PRT(model_name, "*****************nna_get_obj_det_result[Start]*****************\n");
#endif
    for (int i = 0; i < det_out.boxNum; i++)
    {
        nn_result->result_det.nn_result_info[i].class_id = det_out.detBBox[i].clsType;
        nn_result->result_det.nn_result_info[i].box.x = det_out.detBBox[i].bbox.x;
        nn_result->result_det.nn_result_info[i].box.y = det_out.detBBox[i].bbox.y;
        nn_result->result_det.nn_result_info[i].box.w = det_out.detBBox[i].bbox.w;
        nn_result->result_det.nn_result_info[i].box.h = det_out.detBBox[i].bbox.h;
        nn_result->result_det.nn_result_info[i].conf = det_out.detBBox[i].conf;
#if NN_DEBUG
        SDK_PRT(model_name, "[%d] class_id = %d\n", i, nn_result->result_det.nn_result_info[i].class_id);
        SDK_PRT(model_name, "[%d] x = %f\n", i, nn_result->result_det.nn_result_info[i].box.x);
        SDK_PRT(model_name, "[%d] y = %f\n", i, nn_result->result_det.nn_result_info[i].box.y);
        SDK_PRT(model_name, "[%d] w = %f\n", i, nn_result->result_det.nn_result_info[i].box.w);
        SDK_PRT(model_name, "[%d] h = %f\n", i, nn_result->result_det.nn_result_info[i].box.h);
        SDK_PRT(model_name, "[%d] conf = %f\n", i, nn_result->result_det.nn_result_info[i].conf);
#endif
    }
#if NN_DEBUG
    SDK_PRT(model_name, "*****************nna_get_obj_det_result[End]*****************\n");
#endif

Exit:
    ret = FH_NNA_DetectRelease(handle_t->tskHdl, &hw_out_info);
    SDK_FUNC_ERROR(model_name, ret);

    return ret;
}

#if defined(FH_FACE_SNAP_ENABLE) || defined(FH_FACE_REC_ENABLE)
FH_SINT32 nna_get_face_snap_result(NN_HANDLE nn_handle, NN_INPUT_DATA nn_input_data, NN_RESULT *nn_result)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;
    NN_HANDLE_t *handle_t = NULL;
    FH_VIDEO_FRAME nn_src;
    FH_NN_OUT_INFO hw_out_info;
    FH_NN_POST_INPUT_INFO post_input_info;
    FH_EXT_DETECTION_FACEDET_FULL_T post_out;
    FH_UINT32 data_format;

    SDK_CHECK_NULL_PTR(model_name, nn_handle);

    handle_t = (NN_HANDLE_t *)nn_handle;
    SDK_CHECK_NULL_PTR(model_name, handle_t);
    SDK_CHECK_NULL_PTR(model_name, nn_result);

    nn_src.size.u32Width = nn_input_data.width;
    nn_src.size.u32Height = nn_input_data.height;

    ret = chooseFormat(nn_input_data.imageType, &data_format);
    SDK_FUNC_ERROR(model_name, ret);

    nn_src.data_format = data_format;
    nn_src.frm_argb8888.data.base = nn_input_data.data.base;
    nn_src.frm_argb8888.data.vbase = nn_input_data.data.vbase;
    nn_src.frm_argb8888.data.size = nn_input_data.data.size;
    nn_src.time_stamp = nn_input_data.timestamp;
    nn_src.frame_id = nn_input_data.frame_id;
    nn_src.pool_id = nn_input_data.pool_id;

    ret = FH_NNA_DetectProcess(handle_t->tskHdl, &nn_src, &hw_out_info);
    SDK_FUNC_ERROR(model_name, ret);

    memset(&post_input_info, 0, sizeof(post_input_info));
    post_input_info.hw_output_mem.base = hw_out_info.hw_output_mem.base;
    post_input_info.hw_output_mem.vbase = hw_out_info.hw_output_mem.vbase;
    post_input_info.hw_output_mem.size = hw_out_info.hw_output_mem.size;
    post_input_info.current_rotate = hw_out_info.current_rotate;
    post_input_info.current_stamp = hw_out_info.current_stamp;
    post_input_info.frame_id = hw_out_info.frame_id;
    ret = FH_NNPost_FACE_DET_FULL_Process(handle_t->model_info->postHdl, &post_input_info, &post_out);
    SDK_FUNC_ERROR_GOTO(model_name, ret);

    nn_result->result_face_snap.nn_result_num = post_out.boxNum;

#if NN_DEBUG
    SDK_PRT(model_name, "*****************nna_get_face_snap_result[Start]*****************\n");
    SDK_PRT(model_name, "*****face_snap num = %d******\n", nn_result->result_face_snap.nn_result_num);
#endif
    for (int i = 0; i < post_out.boxNum; i++)
    {
        nn_result->result_face_snap.nn_result_info[i].class_id = post_out.detBBox[i].clsType;
        nn_result->result_face_snap.nn_result_info[i].box.x = post_out.detBBox[i].bbox.x;
        nn_result->result_face_snap.nn_result_info[i].box.y = post_out.detBBox[i].bbox.y;
        nn_result->result_face_snap.nn_result_info[i].box.w = post_out.detBBox[i].bbox.w;
        nn_result->result_face_snap.nn_result_info[i].box.h = post_out.detBBox[i].bbox.h;
        nn_result->result_face_snap.nn_result_info[i].conf = post_out.detBBox[i].conf;
        for (int j = 0; j < FH_FACE_LANDMARK_PTS_NUM; j++)
        {
            nn_result->result_face_snap.nn_result_info[i].landmark[j].x = post_out.detBBox[i].landmark[j].x;
            nn_result->result_face_snap.nn_result_info[i].landmark[j].y = post_out.detBBox[i].landmark[j].y;
            nn_result->result_face_snap.nn_result_info[i].occlude[j] = post_out.detBBox[i].occlude[j];
        }
        nn_result->result_face_snap.nn_result_info[i].quality = post_out.detBBox[i].quality;
#if NN_DEBUG
        SDK_PRT(model_name, "[%d] class_id = %d\n", i, nn_result->result_face_snap.nn_result_info[i].class_id);
        SDK_PRT(model_name, "[%d] x = %f\n", i, nn_result->result_face_snap.nn_result_info[i].box.x);
        SDK_PRT(model_name, "[%d] y = %f\n", i, nn_result->result_face_snap.nn_result_info[i].box.y);
        SDK_PRT(model_name, "[%d] w = %f\n", i, nn_result->result_face_snap.nn_result_info[i].box.w);
        SDK_PRT(model_name, "[%d] h = %f\n", i, nn_result->result_face_snap.nn_result_info[i].box.h);
        SDK_PRT(model_name, "[%d] conf = %f\n", i, nn_result->result_face_snap.nn_result_info[i].conf);
        for (int j = 0; j < FH_FACE_LANDMARK_PTS_NUM; j++)
        {
            SDK_PRT(model_name, "[%d] landmark[%d] = (x = %f, y= %f) occlude = %f\n", i, j,
                    nn_result->result_face_snap.nn_result_info[i].landmark[j].x,
                    nn_result->result_face_snap.nn_result_info[i].landmark[j].y,
                    nn_result->result_face_snap.nn_result_info[i].occlude[j]);
        }
        SDK_PRT(model_name, "[%d] quality = %f\n\n", i, nn_result->result_face_snap.nn_result_info[i].quality);
#endif
    }
#if NN_DEBUG
    SDK_PRT(model_name, "*****************nna_get_face_snap_result[End]*****************\n");
#endif

Exit:
    ret = FH_NNA_DetectRelease(handle_t->tskHdl, &hw_out_info);
    SDK_FUNC_ERROR(model_name, ret);

    return ret;
}
#endif

#if defined(FH_FACE_REC_ENABLE)
FH_SINT32 nna_get_face_rec_result(NN_HANDLE nn_handle, NN_INPUT_DATA nn_input_data, NN_RESULT *nn_result)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;
    NN_HANDLE_t *handle_t = NULL;
    FH_VIDEO_FRAME nn_src;
    FH_NN_OUT_INFO hw_out_info;
    FH_NN_POST_INPUT_INFO post_input_info;
    FH_EXT_FACEREC_EMBEDDING_T post_out;
    FH_UINT32 data_format;

    SDK_CHECK_NULL_PTR(model_name, nn_handle);

    handle_t = (NN_HANDLE_t *)nn_handle;
    SDK_CHECK_NULL_PTR(model_name, handle_t);
    SDK_CHECK_NULL_PTR(model_name, nn_result);

    nn_src.size.u32Width = nn_input_data.width;
    nn_src.size.u32Height = nn_input_data.height;

    ret = chooseFormat(nn_input_data.imageType, &data_format);
    SDK_FUNC_ERROR(model_name, ret);

    nn_src.data_format = data_format;
    nn_src.frm_argb8888.data.base = nn_input_data.data.base;
    nn_src.frm_argb8888.data.vbase = nn_input_data.data.vbase;
    nn_src.frm_argb8888.data.size = nn_input_data.data.size;
    nn_src.time_stamp = nn_input_data.timestamp;
    nn_src.frame_id = nn_input_data.frame_id;
    nn_src.pool_id = nn_input_data.pool_id;

    ret = FH_NNA_DetectProcess(handle_t->tskHdl, &nn_src, &hw_out_info);
    SDK_FUNC_ERROR(model_name, ret);

    memset(&post_input_info, 0, sizeof(post_input_info));
    post_input_info.hw_output_mem.base = hw_out_info.hw_output_mem.base;
    post_input_info.hw_output_mem.vbase = hw_out_info.hw_output_mem.vbase;
    post_input_info.hw_output_mem.size = hw_out_info.hw_output_mem.size;
    post_input_info.current_rotate = hw_out_info.current_rotate;
    post_input_info.current_stamp = hw_out_info.current_stamp;
    post_input_info.frame_id = hw_out_info.frame_id;
    ret = FH_NNPost_FACEREC_Process(handle_t->model_info->postHdl, &post_input_info, &post_out);
    SDK_FUNC_ERROR_GOTO(model_name, ret);

#if NN_DEBUG
    SDK_PRT(model_name, "*****************nna_get_face_rec_result[Start]*****************\n");
#endif
    nn_result->result_face_rec.frame_id = post_out.frame_id;
    nn_result->result_face_rec.time_stamp = post_out.time_stamp;
    nn_result->result_face_rec.scale = post_out.scale;
    nn_result->result_face_rec.zero_point = post_out.zero_point;
    memcpy(nn_result->result_face_rec.val, post_out.val, post_out.length);
    nn_result->result_face_rec.length = post_out.length;
#if NN_DEBUG
    SDK_PRT(model_name, "frame_id = %lld\n", nn_result->result_face_rec.frame_id);
    SDK_PRT(model_name, "time_stamp = %lld\n", nn_result->result_face_rec.time_stamp);
    SDK_PRT(model_name, "scale = %f\n", nn_result->result_face_rec.scale);
    SDK_PRT(model_name, "zero_point = %d\n", nn_result->result_face_rec.zero_point);
    SDK_PRT(model_name, "length = %d\n", nn_result->result_face_rec.length);
    SDK_PRT(model_name, "*****************nna_get_face_rec_result[End]*****************\n");
#endif

Exit:
    ret = FH_NNA_DetectRelease(handle_t->tskHdl, &hw_out_info);
    SDK_FUNC_ERROR(model_name, ret);

    return ret;
}
#endif

#if defined(FH_GESTURE_REC_ENABLE)
FH_SINT32 nna_get_ges_det_result(NN_HANDLE nn_handle, NN_INPUT_DATA nn_input_data, NN_RESULT *nn_result)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;
    NN_HANDLE_t *handle_t = NULL;
    FH_VIDEO_FRAME nn_src;
    FH_NN_OUT_INFO hw_out_info;
    FH_NN_POST_INPUT_INFO post_input_info;
    FH_EXT_GESTURE_DET_OUT_ALGST post_out;
    FH_UINT32 data_format;

    SDK_CHECK_NULL_PTR(model_name, nn_handle);

    handle_t = (NN_HANDLE_t *)nn_handle;
    SDK_CHECK_NULL_PTR(model_name, handle_t);
    SDK_CHECK_NULL_PTR(model_name, nn_result);

    nn_src.size.u32Width = nn_input_data.width;
    nn_src.size.u32Height = nn_input_data.height;

    ret = chooseFormat(nn_input_data.imageType, &data_format);
    SDK_FUNC_ERROR(model_name, ret);

    nn_src.data_format = data_format;
    nn_src.frm_argb8888.data.base = nn_input_data.data.base;
    nn_src.frm_argb8888.data.vbase = nn_input_data.data.vbase;
    nn_src.frm_argb8888.data.size = nn_input_data.data.size;
    nn_src.time_stamp = nn_input_data.timestamp;
    nn_src.frame_id = nn_input_data.frame_id;
    nn_src.pool_id = nn_input_data.pool_id;

    ret = FH_NNA_DetectProcess(handle_t->tskHdl, &nn_src, &hw_out_info);
    SDK_FUNC_ERROR(model_name, ret);

    memset(&post_input_info, 0, sizeof(post_input_info));
    post_input_info.hw_output_mem.base = hw_out_info.hw_output_mem.base;
    post_input_info.hw_output_mem.vbase = hw_out_info.hw_output_mem.vbase;
    post_input_info.hw_output_mem.size = hw_out_info.hw_output_mem.size;
    post_input_info.current_rotate = hw_out_info.current_rotate;
    post_input_info.current_stamp = hw_out_info.current_stamp;
    post_input_info.frame_id = hw_out_info.frame_id;

    ret = FH_NNPost_GESTURE_DET_Process(handle_t->model_info->postHdl, &post_input_info, &post_out);
    SDK_FUNC_ERROR_GOTO(model_name, ret);

#if NN_DEBUG
    SDK_PRT(model_name, "##############################################nna_get_gesture_det_result[start]###################################################\n");
#endif
    nn_result->result_gesture_det.frame_id = post_out.frame_id;
    nn_result->result_gesture_det.time_stamp = post_out.time_stamp;
    nn_result->result_gesture_det.nn_result_num = post_out.boxNum;

    for (int i = 0; i < nn_result->result_gesture_det.nn_result_num; i++)
    {
        nn_result->result_gesture_det.nn_result_info[i].class_id = post_out.detBBox[i].clsType;
        nn_result->result_gesture_det.nn_result_info[i].conf = post_out.detBBox[i].conf;
        nn_result->result_gesture_det.nn_result_info[i].box.x = post_out.detBBox[i].bbox.x;
        nn_result->result_gesture_det.nn_result_info[i].box.y = post_out.detBBox[i].bbox.y;
        nn_result->result_gesture_det.nn_result_info[i].box.w = post_out.detBBox[i].bbox.w;
        nn_result->result_gesture_det.nn_result_info[i].box.h = post_out.detBBox[i].bbox.h;
    }
#if NN_DEBUG
    SDK_PRT(model_name, "frame_id = %lld\n", nn_result->result_gesture_det.frame_id);
    SDK_PRT(model_name, "time_stamp = %lld\n", nn_result->result_gesture_det.time_stamp);
    SDK_PRT(model_name, "boxNum = %d\n", nn_result->result_gesture_det.nn_result_num);
    for (int i = 0; i < nn_result->result_gesture_det.nn_result_num; i++)
    {
        SDK_PRT(model_name, "[%d] nn_result->result_gesture_det.nn_result_info[i].class_id = %d\n", i, nn_result->result_gesture_det.nn_result_info[i].class_id);
        SDK_PRT(model_name, "[%d] nn_result->result_gesture_det.nn_result_info[i].conf = %f\n", i, nn_result->result_gesture_det.nn_result_info[i].conf);
        SDK_PRT(model_name, "[%d] nn_result->result_gesture_det.nn_result_info[i].box.x = %f\n", i, nn_result->result_gesture_det.nn_result_info[i].box.x);
        SDK_PRT(model_name, "[%d] nn_result->result_gesture_det.nn_result_info[i].box.y = %f\n", i, nn_result->result_gesture_det.nn_result_info[i].box.y);
        SDK_PRT(model_name, "[%d] nn_result->result_gesture_det.nn_result_info[i].box.w = %f\n", i, nn_result->result_gesture_det.nn_result_info[i].box.w);
        SDK_PRT(model_name, "[%d] nn_result->result_gesture_det.nn_result_info[i].box.h = %f\n", i, nn_result->result_gesture_det.nn_result_info[i].box.h);
    }
#endif

Exit:
    ret = FH_NNA_DetectRelease(handle_t->tskHdl, &hw_out_info);
    SDK_FUNC_ERROR(model_name, ret);

    return ret;
}

FH_SINT32 nna_get_ges_rec_result(NN_HANDLE nn_handle, NN_INPUT_DATA nn_input_data, NN_RESULT *nn_result)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;
    NN_HANDLE_t *handle_t = NULL;
    FH_VIDEO_FRAME nn_src;
    FH_NN_OUT_INFO hw_out_info;
    FH_NN_POST_INPUT_INFO post_input_info;
    FH_EXT_GESTUREREC_OUT_ALGST post_out;
    FH_UINT32 data_format;

    SDK_CHECK_NULL_PTR(model_name, nn_handle);

    handle_t = (NN_HANDLE_t *)nn_handle;
    SDK_CHECK_NULL_PTR(model_name, handle_t);
    SDK_CHECK_NULL_PTR(model_name, nn_result);

    nn_src.size.u32Width = nn_input_data.width;
    nn_src.size.u32Height = nn_input_data.height;

    ret = chooseFormat(nn_input_data.imageType, &data_format);
    SDK_FUNC_ERROR(model_name, ret);

    nn_src.data_format = data_format;
    nn_src.frm_argb8888.data.base = nn_input_data.data.base;
    nn_src.frm_argb8888.data.vbase = nn_input_data.data.vbase;
    nn_src.frm_argb8888.data.size = nn_input_data.data.size;
    nn_src.time_stamp = nn_input_data.timestamp;
    nn_src.frame_id = nn_input_data.frame_id;
    nn_src.pool_id = nn_input_data.pool_id;

    ret = FH_NNA_DetectProcess(handle_t->tskHdl, &nn_src, &hw_out_info);
    SDK_FUNC_ERROR(model_name, ret);

    memset(&post_input_info, 0, sizeof(post_input_info));
    post_input_info.hw_output_mem.base = hw_out_info.hw_output_mem.base;
    post_input_info.hw_output_mem.vbase = hw_out_info.hw_output_mem.vbase;
    post_input_info.hw_output_mem.size = hw_out_info.hw_output_mem.size;
    post_input_info.current_rotate = hw_out_info.current_rotate;
    post_input_info.current_stamp = hw_out_info.current_stamp;
    post_input_info.frame_id = hw_out_info.frame_id;

    ret = FH_NNPost_GESTUREREC_Process(handle_t->model_info->postHdl, &post_input_info, &post_out);
    SDK_FUNC_ERROR_GOTO(model_name, ret);

#if NN_DEBUG
    SDK_PRT(model_name, "#############################################nna_get_gesture_rec_result[start]###########################################\n");
#endif
    nn_result->result_gesture_rec.frame_id = post_out.frame_id;
    nn_result->result_gesture_rec.time_stamp = post_out.time_stamp;
    nn_result->result_gesture_rec.type_info = post_out.type_id;
    nn_result->result_gesture_rec.conf = post_out.prob;

#if NN_DEBUG
    SDK_PRT(model_name, "frame_id = %lld\n", nn_result->result_gesture_rec.frame_id);
    SDK_PRT(model_name, "time_stamp = %lld\n", nn_result->result_gesture_rec.time_stamp);
    SDK_PRT(model_name, "type_id = %d\n", nn_result->result_gesture_rec.type_info);
    SDK_PRT(model_name, "conf = %f\n", nn_result->result_gesture_rec.conf);
#endif

Exit:
    ret = FH_NNA_DetectRelease(handle_t->tskHdl, &hw_out_info);
    SDK_FUNC_ERROR(model_name, ret);

    return ret;
}
#endif

#if defined(FH_LP_REC_ENABLE)
FH_SINT32 get_lpdet_result(NN_HANDLE nn_handle, NN_INPUT_DATA nn_input_data, NN_RESULT *nn_result)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;
    NN_HANDLE_t *handle_t = NULL;
    FH_VIDEO_FRAME nn_src;
    FH_NN_OUT_INFO hw_out_info;
    FH_NN_POST_INPUT_INFO post_input_info;
    FH_EXT_DETECTION_LP_DET_FULL_T post_out;
    FH_UINT32 data_format;

    SDK_CHECK_NULL_PTR(model_name, nn_handle);

    handle_t = (NN_HANDLE_t *)nn_handle;
    SDK_CHECK_NULL_PTR(model_name, handle_t);
    SDK_CHECK_NULL_PTR(model_name, nn_result);

    nn_src.size.u32Width = nn_input_data.width;
    nn_src.size.u32Height = nn_input_data.height;

    ret = chooseFormat(nn_input_data.imageType, &data_format);
    SDK_FUNC_ERROR(model_name, ret);

    nn_src.data_format = data_format;
    nn_src.frm_argb8888.data.base = nn_input_data.data.base;
    nn_src.frm_argb8888.data.vbase = nn_input_data.data.vbase;
    nn_src.frm_argb8888.data.size = nn_input_data.data.size;
    nn_src.time_stamp = nn_input_data.timestamp;
    nn_src.frame_id = nn_input_data.frame_id;
    nn_src.pool_id = nn_input_data.pool_id;

    ret = FH_NNA_DetectProcess(handle_t->tskHdl, &nn_src, &hw_out_info);
    SDK_FUNC_ERROR(model_name, ret);

    memset(&post_input_info, 0, sizeof(post_input_info));
    post_input_info.hw_output_mem.base = hw_out_info.hw_output_mem.base;
    post_input_info.hw_output_mem.vbase = hw_out_info.hw_output_mem.vbase;
    post_input_info.hw_output_mem.size = hw_out_info.hw_output_mem.size;
    post_input_info.current_rotate = hw_out_info.current_rotate;
    post_input_info.current_stamp = hw_out_info.current_stamp;
    post_input_info.frame_id = hw_out_info.frame_id;
    ret = FH_NNPost_LP_DET_FULL_Process(handle_t->model_info->postHdl, &post_input_info, &post_out);
    SDK_FUNC_ERROR(model_name, ret);

#if NN_DEBUG
    SDK_PRT(model_name, "#################################nna_get_lpdet_result[Start]###########################\n");
#endif
    nn_result->result_lp_det.frame_id = post_out.frame_id;
    nn_result->result_lp_det.time_stamp = post_out.time_stamp;
    nn_result->result_lp_det.boxNum = post_out.boxNum;
    for (int i = 0; i < nn_result->result_lp_det.boxNum; i++)
    {
        nn_result->result_lp_det.detBBox[i].class_id = post_out.detBBox[i].clsType;
        nn_result->result_lp_det.detBBox[i].conf = post_out.detBBox[i].conf;
        nn_result->result_lp_det.detBBox[i].quality = post_out.detBBox[i].quality;
        nn_result->result_lp_det.detBBox[i].box.x = post_out.detBBox[i].bbox.x;
        nn_result->result_lp_det.detBBox[i].box.y = post_out.detBBox[i].bbox.y;
        nn_result->result_lp_det.detBBox[i].box.h = post_out.detBBox[i].bbox.h;
        nn_result->result_lp_det.detBBox[i].box.w = post_out.detBBox[i].bbox.w;
        for (int j = 0; j < FH_MAX_LPD_NUM; j++)
        {
            nn_result->result_lp_det.detBBox[i].landmark[j].x = post_out.detBBox[i].landmark[j].x;
            nn_result->result_lp_det.detBBox[i].landmark[j].y = post_out.detBBox[i].landmark[j].y;
            nn_result->result_lp_det.detBBox[i].occlude[j] = post_out.detBBox[i].occlude[j];
        }
#if NN_DEBUG
        SDK_PRT(model_name, "[%d] class_id = %d\n", i, nn_result->result_lp_det.detBBox[i].class_id);
        SDK_PRT(model_name, "[%d] conf = %f\n", i, nn_result->result_lp_det.detBBox[i].conf);
        SDK_PRT(model_name, "[%d] quality = %f\n", i, nn_result->result_lp_det.detBBox[i].quality);
        SDK_PRT(model_name, "[%d] boxNum = %d\n", i, nn_result->result_lp_det.boxNum);
        SDK_PRT(model_name, "[%d] x = %f\n", i, nn_result->result_lp_det.detBBox[i].box.x);
        SDK_PRT(model_name, "[%d] y = %f\n", i, nn_result->result_lp_det.detBBox[i].box.y);
        SDK_PRT(model_name, "[%d] w = %f\n", i, nn_result->result_lp_det.detBBox[i].box.w);
        SDK_PRT(model_name, "[%d] h = %f\n", i, nn_result->result_lp_det.detBBox[i].box.h);
        for (int j = 0; j < FH_MAX_LPD_NUM; j++)
        {
            SDK_PRT(model_name, "[%d] landmark[%d] = (x = %f, y = %f) occlude = %f\n", i, j,
                    nn_result->result_lp_det.detBBox[i].landmark[j].x,
                    nn_result->result_lp_det.detBBox[i].landmark[j].y,
                    nn_result->result_lp_det.detBBox[i].occlude[j]);
        }
#endif
    }
#if NN_DEBUG
    SDK_PRT(model_name, "###############################nna_get_lpdet_result[End]################################\n");
#endif

Exit:
    ret = FH_NNA_DetectRelease(handle_t->tskHdl, &hw_out_info);
    SDK_FUNC_ERROR(model_name, ret);

    return ret;
}

FH_SINT32 get_lprec_result(NN_HANDLE nn_handle, NN_INPUT_DATA nn_input_data, NN_RESULT *nn_result)
{
    FH_SINT32 ret = FH_SDK_SUCCESS;
    NN_HANDLE_t *handle_t = NULL;
    FH_VIDEO_FRAME nn_src;
    FH_NN_OUT_INFO hw_out_info;
    FH_NN_POST_INPUT_INFO post_input_info;
    FH_EXT_LPR_OUT_ALGST post_out;
    FH_UINT32 data_format;

    SDK_CHECK_NULL_PTR(model_name, nn_handle);

    handle_t = (NN_HANDLE_t *)nn_handle;
    SDK_CHECK_NULL_PTR(model_name, handle_t);
    SDK_CHECK_NULL_PTR(model_name, nn_result);

    nn_src.size.u32Width = nn_input_data.width;
    nn_src.size.u32Height = nn_input_data.height;

    ret = chooseFormat(nn_input_data.imageType, &data_format);
    SDK_FUNC_ERROR(model_name, ret);

    nn_src.data_format = data_format;
    nn_src.frm_argb8888.data.base = nn_input_data.data.base;
    nn_src.frm_argb8888.data.vbase = nn_input_data.data.vbase;
    nn_src.frm_argb8888.data.size = nn_input_data.data.size;
    nn_src.time_stamp = nn_input_data.timestamp;
    nn_src.frame_id = nn_input_data.frame_id;
    nn_src.pool_id = nn_input_data.pool_id;

    ret = FH_NNA_DetectProcess(handle_t->tskHdl, &nn_src, &hw_out_info);
    SDK_FUNC_ERROR(model_name, ret);

    memset(&post_input_info, 0, sizeof(post_input_info));
    post_input_info.hw_output_mem.base = hw_out_info.hw_output_mem.base;
    post_input_info.hw_output_mem.vbase = hw_out_info.hw_output_mem.vbase;
    post_input_info.hw_output_mem.size = hw_out_info.hw_output_mem.size;
    post_input_info.current_rotate = hw_out_info.current_rotate;
    post_input_info.current_stamp = hw_out_info.current_stamp;
    post_input_info.frame_id = hw_out_info.frame_id;
    ret = FH_NNPost_LPREC_Process(handle_t->model_info->postHdl, &post_input_info, &post_out);
    SDK_FUNC_ERROR(model_name, ret);

#if NN_DEBUG
    SDK_PRT(model_name, "#################nna_get_lprec_result[Start]##################\n");
#endif
    nn_result->result_lp_rec.frame_id = post_out.frame_id;
    nn_result->result_lp_rec.time_stamp = post_out.time_stamp;
    nn_result->result_lp_rec.plate_len = post_out.plate_len;
    memcpy(nn_result->result_lp_rec.val_dec, post_out.val_dec, post_out.plate_len * sizeof(FH_UINT8));
    memcpy(nn_result->result_lp_rec.prob_dec, post_out.prob_dec, post_out.plate_len * sizeof(FH_FLOAT));
#if NN_DEBUG
    SDK_PRT(model_name, "frame_id = %lld\n", nn_result->result_lp_rec.frame_id);
    SDK_PRT(model_name, "time_stamp = %lld\n", nn_result->result_lp_rec.time_stamp);
    SDK_PRT(model_name, "plate_len = %d\n", nn_result->result_lp_rec.plate_len);
    for (int i = 0; i < FH_LP_LENGTH; i++)
    {
        SDK_PRT(model_name, "prob_dec[%d] = %f\n", i, nn_result->result_lp_rec.prob_dec[i]);
        SDK_PRT(model_name, "val_dec[%d] = %d\n", i, nn_result->result_lp_rec.val_dec[i]);
    }
    SDK_PRT(model_name, "################nna_get_lprec_result[end]####################\n");
#endif

Exit:
    ret = FH_NNA_DetectRelease(handle_t->tskHdl, &hw_out_info);
    SDK_FUNC_ERROR(model_name, ret);

    return ret;
}
#endif