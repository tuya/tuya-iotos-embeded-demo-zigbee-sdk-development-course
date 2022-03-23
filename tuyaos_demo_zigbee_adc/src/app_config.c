/*************************************************************************************/
/* Automatically-generated file. Do not edit! */
/*************************************************************************************/

#include "app_config.h"
#include "tuya_iot_config.h"
#include "tal_firmware_cfg.h"


OPERATE_RET tuya_app_firmware_config(VOID_T)
{
    CHAR_T *model_id = MODEL_ID;
    CHAR_T *prefix = PID_ID_PFEFIX;
    CHAR_T *pid = PRODUCTOR_ID;
    CHAR_T *name = FIRMWARE_NAME;
    return tal_firmware_info_init(model_id, prefix, pid, name, FIRMWARE_VER, MANUFACTURER_ID, IMAGE_TYPE);
}


OPERATE_RET tuya_sdk_information_config(VOID_T)
{
    CHAR_T *sdk_ver = IOT_SDK_VER;
    CHAR_T *kcfg_ver = "0.0.0";
    CHAR_T *kernel_ver = KERNEL_VERSION;
    return tal_tuyaos_info_init(sdk_ver, kcfg_ver, kernel_ver);
}


