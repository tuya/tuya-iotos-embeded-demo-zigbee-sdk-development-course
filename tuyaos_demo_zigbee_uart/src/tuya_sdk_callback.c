/*
 * @FileName: your project
 * @Author: Tuya
 * @Email:
 * @LastEditors: Tuya
 * @Date: 2022-03-07 14:24:24
 * @LastEditTime: 2022-03-29 11:35:59
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company:  http://www.tuya.com
 * @Description:
 */
#include "tal_uart.h"
#include "tal_log.h"
#include "tal_system.h"
#include "tuya_uart_demo.h"
#include "app_config.h"
#include "tal_data_send.h"
#include "tal_network_mgr.h"
#include "tal_zcl_identify.h"
#include "tal_data_receive.h"
#include "tal_attribute_rw.h"
#include "tal_firmware_cfg.h"
#include "tal_endpoint_register.h"

#define BUFFER_SIZE 256

#ifndef GET_ARRAY_LEN
#define GET_ARRAY_LEN(x) (sizeof(x) / sizeof(x[0]))
#endif

#define IDENTIFY_ATTR_LIST                                                                                                     \
    {0x0000, ATTR_INT16U_ATTRIBUTE_TYPE, 2, (ATTR_MASK_TOKEN_FAST),0, ((UINT8_T *)0x0005)},   /* 12 / Identify / identify time*/ \
        {0xFFFD, ATTR_INT16U_ATTRIBUTE_TYPE, 2, (ATTR_MASK_READABLE), 0,((UINT8_T *)0x0001)}, /* 13 / Identify / cluster revision*/

#define IDENTIFY_CLIENT_ATTR_LIST \
    {0xFFFD, ATTR_INT16U_ATTRIBUTE_TYPE, 2, (ATTR_MASK_CLIENT),0, ((UINT8_T *)0x0001)}, /* 13 / Identify / cluster revision*/

#define ON_OFF_LIGHT_ATTR_LIST                                                                                      \
    {0x0000, ATTR_BOOLEAN_ATTRIBUTE_TYPE, 1, (ATTR_MASK_READABLE),0, (UINT8_T *)0x00},      /* 20 / On/off / on/off*/ \
        {0xFFFD, ATTR_INT16U_ATTRIBUTE_TYPE, 2, (ATTR_MASK_READABLE), 0,(UINT8_T *)0x0002}, /* 24 / On/off / cluster revision*/

#define GROUP_ATTR_LIST                                                                                                   \
    {0x0000, ATTR_BITMAP8_ATTRIBUTE_TYPE, 1, (ATTR_MASK_READABLE), 0,(UINT8_T *)0x00},      /* 12 / Groups / name support*/ \
        {0xFFFD, ATTR_INT16U_ATTRIBUTE_TYPE, 2, (ATTR_MASK_READABLE), 0,(UINT8_T *)0x0002}, /* 13 / Groups / cluster revision*/

#define SCENE_ATTR_LIST                                                                                                    \
    {0x0000, ATTR_INT8U_ATTRIBUTE_TYPE, 1, (ATTR_MASK_READABLE),0, (UINT8_T *)0x00},        /* 15 / Scenes / scene count*/   \
        {0x0001, ATTR_INT8U_ATTRIBUTE_TYPE, 1, (ATTR_MASK_READABLE),0, (UINT8_T *)0x00},    /* 16 / Scenes / current scene*/ \
        {0x0002, ATTR_INT16U_ATTRIBUTE_TYPE, 2, (ATTR_MASK_READABLE), 0,(UINT8_T *)0x0000}, /* 16 / Scenes / current group*/ \
        {0x0003, ATTR_BOOLEAN_ATTRIBUTE_TYPE, 1, (ATTR_MASK_READABLE),0, (UINT8_T *)0x00},  /* 17 / Scenes / scene valid*/   \
        {0x0004, ATTR_BITMAP8_ATTRIBUTE_TYPE, 1, (ATTR_MASK_READABLE), 0,(UINT8_T *)0x00},  /* 18 / Scenes / name support*/  \
        {0xFFFD, ATTR_INT16U_ATTRIBUTE_TYPE, 2, (ATTR_MASK_READABLE), 0,(UINT8_T *)0x0002}, /* 19 / Scenes / cluster revision*/

#define LEVEL_CONTROL_ATTR_LIST                                                                                                             \
    {0x0000, ATTR_INT8U_ATTRIBUTE_TYPE, 1, (ATTR_MASK_TOKEN_FAST), 0,(UINT8_T *)0x01},        /* 20 / Level Control / Current Level*/         \
        {0xF000, ATTR_INT16U_ATTRIBUTE_TYPE, 2, (ATTR_MASK_TOKEN_FAST),0, (UINT8_T *)0x0001}, /* 20 / Private Level Control / Current Level*/ \
        {0xFFFD, ATTR_INT16U_ATTRIBUTE_TYPE, 2, (ATTR_MASK_READABLE),0, (UINT8_T *)0x0001},

#define COLOR_CONTROL_ATTR_LIST                                                                                                      \
    {0xF000, ATTR_INT8U_ATTRIBUTE_TYPE, 1, (ATTR_MASK_READABLE),0, (UINT8_T *)0x00},      /* 20 / Private Color Control / color mode*/ \
        {0xF102, ATTR_ARRAY_ATTRIBUTE_TYPE, 13, (ATTR_MASK_READABLE),0, (UINT8_T *)0x00}, /* app  data*/                               \
        {0xFFFD, ATTR_INT16U_ATTRIBUTE_TYPE, 2, (ATTR_MASK_READABLE),0, (UINT8_T *)0x0001},


#define  ZLL_ATTR_LIST  \
    { 0xFFFD, ATTR_INT16U_ATTRIBUTE_TYPE, 2, 0, 0, (UINT8_T*)0x0002 },


// application attribute
const TAL_ATTR_T identify_attr_list[] = {
    IDENTIFY_ATTR_LIST};

const TAL_ATTR_T onoff_attr_list[] = {
    ON_OFF_LIGHT_ATTR_LIST};

const TAL_ATTR_T group_attr_list[] = {
    GROUP_ATTR_LIST};

const TAL_ATTR_T scene_attr_list[] = {
    SCENE_ATTR_LIST};

const TAL_ATTR_T level_attr_list[] = {
    LEVEL_CONTROL_ATTR_LIST};

const TAL_ATTR_T color_attr_list[] = {
    COLOR_CONTROL_ATTR_LIST};

const TAL_ATTR_T zll_attr_list[] = {
    ZLL_ATTR_LIST};


#define DEF_CLUSTER_ELECTRICAL_MEASUREMENT_CLUSTER_ID(a) \
    {CLUSTER_ELECTRICAL_MEASUREMENT_CLUSTER_ID, (TAL_ATTR_T *)&((a)[0]), GET_ARRAY_LEN((a))},

#define DEF_CLUSTER_IDENTIFY_CLUSTER_ID(a) \
    {CLUSTER_IDENTIFY_CLUSTER_ID, (TAL_ATTR_T *)&((a)[0]), GET_ARRAY_LEN((a))},

#define DEF_CLUSTER_ON_OFF_CLUSTER_ID(a) \
    {CLUSTER_ON_OFF_CLUSTER_ID, (TAL_ATTR_T *)&((a)[0]), GET_ARRAY_LEN((a))},

#define DEF_CLUSTER_GROUPS_CLUSTER_ID(a) \
    {CLUSTER_GROUPS_CLUSTER_ID, (TAL_ATTR_T *)&((a)[0]), GET_ARRAY_LEN((a))},

#define DEF_CLUSTER_SCENES_CLUSTER_ID(a) \
    {CLUSTER_SCENES_CLUSTER_ID, (TAL_ATTR_T *)&((a)[0]), GET_ARRAY_LEN((a))},

#define DEF_CLUSTER_LEVEL_CONTROL_CLUSTER_ID(a) \
    {CLUSTER_LEVEL_CONTROL_CLUSTER_ID, (TAL_ATTR_T *)&((a)[0]), GET_ARRAY_LEN((a))},

#define DEF_CLUSTER_COLOR_CONTROL_CLUSTER_ID(a) \
    {CLUSTER_COLOR_CONTROL_CLUSTER_ID, (TAL_ATTR_T *)&((a)[0]), GET_ARRAY_LEN((a))},

#define DEF_CLUSTER_ZLL_COMMISSIONING_CLUSTER_ID(a) \
    { CLUSTER_ZLL_COMMISSIONING_CLUSTER_ID, (TAL_ATTR_T *)&((a)[0]), GET_ARRAY_LEN((a)) },


// server cluster
const TAL_CLUSTER_T app_server_cluster_list[] = {
    DEF_CLUSTER_IDENTIFY_CLUSTER_ID(identify_attr_list)
        DEF_CLUSTER_GROUPS_CLUSTER_ID(group_attr_list)
            DEF_CLUSTER_SCENES_CLUSTER_ID(scene_attr_list)
                DEF_CLUSTER_ON_OFF_CLUSTER_ID(onoff_attr_list)
                    DEF_CLUSTER_LEVEL_CONTROL_CLUSTER_ID(level_attr_list)
                        DEF_CLUSTER_COLOR_CONTROL_CLUSTER_ID(color_attr_list)
                            DEF_CLUSTER_ZLL_COMMISSIONING_CLUSTER_ID(zll_attr_list)};


#define SERVER_CLUSTER_NUM GET_ARRAY_LEN(app_server_cluster_list)

TAL_ENDPOINT_T dev_endpoint_desc[] = {
    {1, ZHA_PROFILE_ID, ZG_DEVICE_ID_ON_OFF_LIGHT, SERVER_CLUSTER_NUM, (TAL_CLUSTER_T *)&app_server_cluster_list[0], 0, NULL},
};



/**
 * @brief zigbee node init
 * 
 * @return VOID_T 
 */
STATIC VOID_T __app_router_node_init(VOID_T)
{
    TAL_ZG_NODE_CFG_T node_config = {
        .node_type = ZG_ROUTER,
        .tx_power = 11,
        .scan_interval = 0,
        .scan_duration = ZG_SCAN_DURATION_3,
    };
    tal_zg_node_config(&node_config);
}



/**
 * @brief
 *
 * @param str
 * @return VOID_T
 */
VOID_T dev_uart_output(IN CONST CHAR_T *str)
{
    // UINT8_T *v_buff = (UINT8_T *)tkl_system_malloc(strlen(str));
    // if (v_buff == NULL) {
    //     return;
    // }

    // tkl_system_memcpy(v_buff, str, strlen(str));
    // tal_uart_write(USER_UART0, v_buff, strlen((CHAR_T *)v_buff));
    // tal_uart_write(USER_UART0, str, strlen(str));

    // tkl_system_free(v_buff);
    // v_buff = NULL;
    tal_uart_write(USER_UART0, str, strlen(str));

}


/**
 * @brief Generally used for peripheral initialation
 *
 * @return OPERATE_RET
 */
OPERATE_RET tuya_init_first(VOID_T)
{
#if (ENABLE_TAL_LOG == 1)
    tal_log_create_manage_and_init(TAL_LOG_LEVEL_DEBUG, 128, dev_uart_output);
#endif
    user_uart_init();
    TAL_PR_DEBUG("/*********first init*********/");
    TAL_PR_DEBUG("USER_UART0 init success!\n");

    return OPRT_OK;
}


OPERATE_RET tuya_init_second(VOID_T)
{
    //initialize firmware infomation
    TAL_PR_DEBUG("Application version:%d", FIRMWARE_VER);

    //register zigbee endpoint
    tal_zg_endpoint_register(dev_endpoint_desc, GET_ARRAY_LEN(dev_endpoint_desc));
    TAL_PR_DEBUG("identify init ret:%d", tal_zg_identify_init());

    //zigbee node configuration
    __app_router_node_init();

    //zigbee joining network configuration
    TAL_ZG_JOIN_CFG_T join_config = {
        .auto_join_power_on_flag = TRUE,
        .auto_join_remote_leave_flag = TRUE,
        .join_timeout = 20000,
    };

    tal_time_sync_period_set(60 * 1000);

    user_uart_task();

    TAL_PR_DEBUG("/*********second init*********/");

    return OPRT_OK;

}


/**
 * @brief Generally used for initialization before manufacturing test
 *
 * @return OPERATE_RET
 */
OPERATE_RET tuya_init_third(VOID_T)
{
    TAL_PR_DEBUG("/*********third init*********/");

    return OPRT_OK;
}

/**
 * @brief Generally used for initialization after manufacturing test
 *
 * @return OPERATE_RET
 */
OPERATE_RET tuya_init_last(VOID_T)
{
    UINT8_T version;

    tal_uart_deinit(USER_UART0);

    TAL_UART_CFG_T uart_cfg = {
        .rx_buffer_size = 256,
        .open_mode = 0,
        {
            .baudrate = 115200,
            .parity = TUYA_UART_PARITY_TYPE_NONE,
            .databits = TUYA_UART_DATA_LEN_8BIT,
            .stopbits = TUYA_UART_STOP_LEN_1BIT,
            .flowctrl = TUYA_UART_FLOWCTRL_NONE,
        }
    };
    tal_uart_init(USER_UART0, &uart_cfg);

    tal_zg_read_attribute(TUYA_PRIMARY_ENDPOINT,
                          CLUSTER_BASIC_CLUSTER_ID,
                          ATTR_APPLICATION_VERSION_ATTRIBUTE_ID,
                          (VOID_T *)&version,
                          sizeof(version));

    TAL_PR_DEBUG("/*********last init %d*********/", version);
    return OPRT_OK;
}


/**
 * @brief user-defined callback interface in main loop.do not block!!!
 *
 * @return OPERATE_RET
 */
OPERATE_RET tuya_main_loop(VOID_T)
{
    return OPRT_OK;
}





