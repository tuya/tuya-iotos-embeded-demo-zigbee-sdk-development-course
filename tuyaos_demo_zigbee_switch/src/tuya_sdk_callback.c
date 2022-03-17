/**
* @file tuya_sdk_callback.c
* @brief Common process - adapter the flash api
* @version 0.1
* @date 2021-10-22
*
* @copyright Copyright 2020-2021 Tuya Inc. All Rights Reserved.
*
*/
#include "tuya_tools.h"

#include "tal_log.h"
#include "tal_gpio.h"
#include "tal_uart.h"
#include "tal_flash.h"
#include "tal_system.h"
#include "tal_memory.h"
#include "tal_sw_timer.h"
#include "tal_zcl_group.h"
#include "tal_zcl_scene.h"
#include "tal_time_sync.h"
#include "tal_heartbeat.h"
#include "tal_data_send.h"
#include "tal_network_mgr.h"
#include "tal_zcl_identify.h"
#include "tal_data_receive.h"
#include "tal_attribute_rw.h"
#include "tal_firmware_cfg.h"
#include "tal_reset_factory.h"
#include "tal_zll_commissioning.h"
#include "tal_endpoint_register.h"

#include "app_common.h"
#include "app_config.h"
#include "app_cluster_on_off.h"


TIMER_ID etimer_key_scan;
TIMER_ID etimer_blink_sw;
TIMER_ID etimer_countdown;
TIMER_ID etimer_join_start_delay;
TIMER_ID etimer_clr_rst_cnt_delay;
TIMER_ID etimer_save_data_delay;
TIMER_ID etimer_mf_blink;
TIMER_ID etimer_power_on_sync;
TIMER_ID etimer_join_end_sync;


#ifdef ENABLE_TAL_LOG
const char *g_net_st_str_test[] = {
    "TAL_ZG_NWK_IDLE",            ///< inner using
    "TAL_ZG_NWK_POWER_ON_LEAVE",  ///< power on and device is not joined network
    "TAL_ZG_NWK_POWER_ON_ONLINE", ///< power on and device is already joined network
    "TAL_ZG_NWK_JOIN_START",      ///< start joining network
    "TAL_ZG_NWK_JOIN_TIMEOUT",    ///< network joining timeout
    "TAL_ZG_NWK_JOIN_OK",         ///< network joined success
    "TAL_ZG_NWK_LOST",            ///< network lost, lost parent
    "TAL_ZG_NWK_REJOIN_OK",       ///< network rejoin ok
    "TAL_ZG_NWK_REMOTE_LEAVE",    ///< remove device by remote device
    "TAL_ZG_NWK_LOCAL_LEAVE",     ///< remove device by local
    "TAL_ZG_NWK_MF_TEST_LEAVE",   ///< remove device by PC test tools
    "TAL_ZG_NWK_ZLL_JOINED",      ///< network joined zll network
    "TAL_ZG_NWK_ZLL_LEAVE",       ///< remove device Zll Reset To Factory New
};
#endif


extern VOID_T app_onoff_count_down_timer_cb(TIMER_ID timer_id, VOID_T *arg);
STATIC light_ctrl_data_t sg_light_ctrl_data;

#ifndef GET_ARRAY_LEN   tal_sw_timer_create(app_onoff)
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
    {1, ZHA_PROFILE_ID, ZG_DEVICE_ID_ON_OFF_SWITCH, SERVER_CLUSTER_NUM, (TAL_CLUSTER_T *)&app_server_cluster_list[0], 0, NULL},
};

STATIC UINT8_T sg_led_status = 0;
light_ctrl_data_t sg_light_ctrl_data;


/**
 * @brief
 *
 */
STATIC void __app_power_on_check_rst_cnt_proc(void)
{
    rst_info_t v_info = {0x00, 0x00};

    TUYA_RESET_REASON_E v_reason = tal_system_get_reset_reason(NULL);

    TAL_PR_DEBUG("Reset Reason: %d", v_reason);
    if (TUYA_RESET_REASON_HW_WDOG == v_reason)
    {
        return;
    }

    // tal_flash_write(0xf0000,);
    tal_flash_read(FLASH_ADDR_RST_CNT, (UCHAR_T *)&v_info, sizeof(rst_info_t));

    if (v_info.is_warm_start != 0xa5)
    {
        v_info.is_warm_start = 0xa5;
        v_info.rst_cnt = 1;
        tal_flash_erase(FLASH_ADDR_RST_CNT, MG21_FLASH_PAGE_SIZE);
        tal_flash_write(FLASH_ADDR_RST_CNT, (UCHAR_T *)&v_info, sizeof(rst_info_t));
        return;
    }

    v_info.rst_cnt++;
    if (v_info.rst_cnt >= 3)
    {
        TAL_PR_DEBUG("join by rst cnt over...");
        tal_sw_timer_start(etimer_join_start_delay, 20, TAL_TIMER_ONCE);

        //5S timeout for clear reset cnt
        tal_sw_timer_start(etimer_clr_rst_cnt_delay, 5000, TAL_TIMER_ONCE);
        return;
    }

    tal_flash_erase(FLASH_ADDR_RST_CNT, MG21_FLASH_PAGE_SIZE);
    tal_flash_write(FLASH_ADDR_RST_CNT, (UCHAR_T *)&v_info, sizeof(rst_info_t));

    //5S timeout for clear reset cnt
    tal_sw_timer_start(etimer_clr_rst_cnt_delay, 5000, TAL_TIMER_ONCE);
}


/**
 * @note join start delay timer hander, join now
 * @param[in]  none
 * @param[out] none
 * @return none
 */
STATIC VOID_T __app_network_join_start_delay_cb(TIMER_ID timer_id, VOID_T *arg)
{
    tal_zg_join_start(ZIGBEE_JOIN_MAX_TIMEOUT);

}

/**
 * @brief
 *
 * @param timer_id
 * @param arg
 * @return STATIC
 */
STATIC VOID_T __app_clr_rst_cnt_delay_cb(TIMER_ID timer_id, VOID_T *arg)
{
    rst_info_t v_info = {0xa5, 0x00};
    tal_flash_erase(FLASH_ADDR_RST_CNT, MG21_FLASH_PAGE_SIZE);
    tal_flash_write(FLASH_ADDR_RST_CNT, (UCHAR_T *)&v_info, sizeof(rst_info_t));
}



ULONG_T app_light_count_down_cnt_get(VOID_T)
{
    return sg_light_ctrl_data.count_down_cnt;
}

VOID_T app_light_count_down_cnt_set(ULONG_T count)
{
    sg_light_ctrl_data.count_down_cnt = count;
}



/**
 * @note: app_onoff_count_down_timer_cb
 * @note: count down cnt 60s to report value
 * @param {none}
 * @return: none
 */
VOID_T app_onoff_count_down_timer_cb(TIMER_ID timer_id, VOID_T *arg)
{
    OPERATE_RET ret = -1;
    ULONG_T cnt = app_light_count_down_cnt_get();
    UINT8_T onoff = FALSE;
    if (cnt > 1)
    {
        cnt--;
        app_light_count_down_cnt_set(cnt);
        if ((cnt % 60) == 0)
        {
            app_onoff_count_down_response(cnt);
        }
    }
    else
    {
        app_light_count_down_cnt_set(0);
        app_light_ctrl_data_switch_get(&onoff);
        onoff = (onoff != FALSE) ? FALSE : TRUE;
        app_light_ctrl_data_switch_set(onoff);
        ret = app_light_ctrl_proc();
        if (ret != OPRT_OK)
        {
            TAL_PR_DEBUG("CountDown process error!");
        }
        app_onoff_count_down_response(0);
        return;
    }
    tal_sw_timer_start(etimer_countdown, 1000, TAL_TIMER_ONCE);
}


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
 * @brief Generally used for peripheral initialization
 *
 * @return OPERATE_RET
 */
OPERATE_RET tuya_init_first(VOID_T)
{
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

    app_button_init();

#if (ENABLE_TAL_LOG == 1)
    tal_log_create_manage_and_init(TAL_LOG_LEVEL_DEBUG, 128, dev_uart_output);
#endif

    TAL_PR_DEBUG("/*********first init*********/");
    return OPRT_OK;
}



/**
 * @brief Generally used for register zigbee device
 *
 * @return OPERATE_RET
 */
OPERATE_RET tuya_init_second(VOID_T)
{
    //initialize firmware infomation
    TAL_PR_DEBUG("Application version:%d", FIRMWARE_VER);

    //creat software timer

    tal_sw_timer_create(app_button_scan_cb, NULL, &etimer_key_scan);

    tal_sw_timer_create(app_onoff_count_down_timer_cb, NULL, &etimer_countdown);

    tal_sw_timer_create(__app_network_join_start_delay_cb, NULL, &etimer_join_start_delay);
    tal_sw_timer_create(__app_clr_rst_cnt_delay_cb, NULL, &etimer_clr_rst_cnt_delay);

    tal_sw_timer_create(app_light_ctrl_save_data_delay_cb, NULL, &etimer_save_data_delay);
    tal_sw_timer_create(app_light_ctrl_power_on_sync_cb, NULL, &etimer_power_on_sync);
    tal_sw_timer_create(app_light_ctrl_join_end_sync_cb, NULL, &etimer_join_end_sync);


    //register zigbee endpoint
    tal_zg_endpoint_register(dev_endpoint_desc, GET_ARRAY_LEN(dev_endpoint_desc));
    TAL_PR_DEBUG("identify init ret:%d", tal_zg_identify_init());
    tal_zll_target_touchlink_init();
    tal_zll_target_rssi_threshold_set(-90);


    //zigbee node configuration
    __app_router_node_init();

    //zigbee joining network configuration
    TAL_ZG_JOIN_CFG_T join_config = {
        .auto_join_power_on_flag = TRUE,
        .auto_join_remote_leave_flag = TRUE,
        .join_timeout = 20000,
    };
    tal_zg_join_config(&join_config);

    tal_time_sync_period_set(60 * 1000);
    __app_power_on_check_rst_cnt_proc();
    TAL_PR_DEBUG("/*********1.0.0 second init*********/");

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

    OPERATE_RET ret;

    ret = app_light_init();

    if (ret != OPRT_OK)
    {
        TAL_PR_DEBUG("light init ERROR");
    }
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

	// tal_heartbeat_period_set(30*000);
    // tal_heartbeat_type_set(HEARTBEAT_APP_VERSION);
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


/**
 * @brief heartbeat report callback
 * NOTE: you can rewrite this API in application layer
 *
 * @param[in]   status: heartbeat report status
 *
 * @return none
 */
VOID_T tal_heartbeat_report_callback(BOOL_T status)
{
    TAL_PR_DEBUG("status: %d", status);
}
/**
 * @brief general message receive callback
 *
 * @param msg
 * @return TAL_MSG_RET_E
 */
TAL_MSG_RET_E tal_zcl_general_msg_recv_callback(TAL_ZCL_MSG_T *msg)
{
    return ZCL_MSG_RET_SUCCESS;
}
/**
 * @brief specific message receive callback
 *
 * @param msg
 * @return TAL_MSG_RET_E
 */
TAL_MSG_RET_E tal_zcl_specific_msg_recv_callback(TAL_ZCL_MSG_T *msg)
{
    TAL_PR_DEBUG("app spec msg cb: cluster 0x%02x, cmd 0x%02x", msg->cluster, msg->command);

    ZIGBEE_CMD_T app_cmd_type = ZIGBEE_CMD_SINGLE;
    if (msg->mode == ZG_UNICAST_MODE)
    {
        app_cmd_type = ZIGBEE_CMD_SINGLE;
        TAL_PR_DEBUG("receive single message");
    }
    else
    {
        app_cmd_type = ZIGBEE_CMD_GROUP;
        TAL_PR_DEBUG("receive group message");
    }

    switch (msg->cluster)
    {
    case CLUSTER_ON_OFF_CLUSTER_ID:
    {
        //handle on/off cluster command
        app_onoff_cluster_handler(msg->command, msg->payload, msg->length, app_cmd_type);
    }
    break;
    case CLUSTER_LEVEL_CONTROL_CLUSTER_ID:
    {
        //handle level control cluster command
        app_level_cluster_handler(msg->command, msg->payload, msg->length, app_cmd_type);
    }
    break;

    case CLUSTER_PRIVATE_TUYA_CLUSTER_ID:
    {
        break;
    }
    default:
        break;
    }

    return ZCL_MSG_RET_SUCCESS;
}


/**
 * @brief pre-save scene callback
 *
 * @param ep_id
 * @param scene_id
 * @param group_id
 * @return VOID_T
 */
VOID_T tal_zg_scene_pre_save_callback(UINT8_T ep_id, UINT8_T scene_id, UINT16_T group_id)
{
    TAL_PR_DEBUG("scene pre save: ep %d, sce %d, gp 0x%02x", ep_id, scene_id, group_id);
}



/**
 * @brief save scene callback(add scene and store scene)
 *
 * @param ep_id
 * @param scene_id
 * @param group_id
 * @param data
 * @return VOID_T
 */
VOID_T tal_zg_scene_save_callback(UINT8_T ep_id, UINT8_T scene_id, UINT16_T group_id, TAL_SCENE_DATA_T *data)
{
    TAL_PR_DEBUG("scene save: ep %d, sce %d, gp 0x%02x, type %d", ep_id, scene_id, group_id, data->type);

    UINT8_T onoff;

    app_light_ctrl_data_switch_get((UINT8_T *)&onoff);

    data->pdata[0] = onoff;

    data->pdata[1] = 0;

    data->len = 1;

    TAL_PR_DEBUG("ADD SCENE DATA LEN = 0x%x", data->len);
    TAL_PR_DEBUG("ADD SCENE DATA : ");

}



/**
 * @brief recall scene callback
 *
 * @param[in]   ep_id:     endpoint id
 * @param[in]   scene_id:  scene id
 * @param[in]   group_id:  group id
 * @param[in]   time100ms: scene transition time(bat:100ms)
 * @param[in]   data:      point to scene data
 * @return  VOID_T
 */
VOID_T tal_zg_scene_recall_callback(UINT8_T ep_id, UINT8_T scene_id, UINT16_T group_id, UINT_T time100ms, TAL_SCENE_DATA_T *data)
{
    TAL_PR_DEBUG("scene recall: ep %d, sce %d, gp 0x%02x, type %d", ep_id, scene_id, group_id, data->type);
    UINT8_T onoff;
 
    UINT8_T proc_flag = TRUE;

    OPERATE_RET ret = 0;

    UINT8_T pre_onoff;

    TAL_PR_DEBUG("RECALL SCENE DATA LEN = %d", data->len);

    onoff = data->pdata[0];

    app_light_ctrl_data_switch_get((UINT8_T *)&pre_onoff);
    ret = app_light_ctrl_data_switch_set(onoff);

    if (pre_onoff != onoff)
    {
        //onoff state has been changed, stop the count down timer, and report onoff
        app_onoff_set_count_down_report_flag_time(TRUE, 1000);
        app_onoff_data_count_down_set(0);
    }
    if (ret == OPRT_OK)
    {
        proc_flag = TRUE;
    }
    //ret = app_light_ctrl_data_mode_set((LIGHT_MODE_E)mode);
    if (ret == OPRT_OK)
    {
        proc_flag = TRUE;
    }

    if (proc_flag)
    {
        app_light_ctrl_proc();
        app_onoff_report_value(QOS_0, 0, onoff);

    }
}



/**
 * @brief add group callback
 *
 * @param[in]   ep_id:    endpoint id
 * @param[in]   group_id: group id
 * @return  VOID_T
 */
VOID_T tal_zg_add_group_callback(UINT8_T ep_id, UINT16_T group_id)
{
    TAL_PR_DEBUG("ep id: %d,group id: %d", ep_id, group_id);
}
/**
 * @brief view group callback
 *
 * @param[in]   ep_id:    endpoint id
 * @param[in]   group_id: group id
 * @return  VOID_T
 */
VOID_T tal_zg_view_group_callback(UINT8_T ep_id, UINT16_T group_id)
{
    TAL_PR_DEBUG("ep id: %d,group id: %d", ep_id, group_id);
}
/**
 * @brief remove group callback
 *
 * @param[in]   ep_id:    endpoint id
 * @param[in]   group_id: group id
 * @return  VOID_T
 */
VOID_T tal_zg_remove_group_callback(UINT8_T ep_id, UINT16_T group_id)
{
    TAL_PR_DEBUG("ep id: %d,group id: %d", ep_id, group_id);
}
/**
 * @brief zigbee network network change callback(user can rewrite this API)
 * 
 * @param[in]   status: network status
 * @return VOID_T
 */
VOID_T tal_zg_nwk_status_changed_callback(TAL_ZG_NWK_STATUS_E status)
{
    switch (status)
    {
    case TAL_ZG_NWK_POWER_ON_LEAVE:
    {
        TAL_PR_DEBUG("power_on_leave---");
        break;
    }
    case TAL_ZG_NWK_POWER_ON_ONLINE:
    {
        TAL_PR_DEBUG("power_on_online---");
        break;
    }
    case TAL_ZG_NWK_JOIN_START:
    {
        TAL_PR_DEBUG("nwk_join_start---");
        //light blink when steering

        netled_status_task();

        break;
    }
    case TAL_ZG_NWK_JOIN_OK:
    {
        TAL_PR_DEBUG("nwk_join_ok---");

        app_light_ctrl_blink_stop();

        break;
    }
    case TAL_ZG_NWK_REJOIN_OK:
    {
        TAL_PR_DEBUG("nwk_rejoin_ok---");
        break;
    }
    case TAL_ZG_NWK_JOIN_TIMEOUT:
    {
        TAL_PR_DEBUG("nwk_join_timeout---");
        app_light_ctrl_blink_stop();

        break;
    }
    case TAL_ZG_NWK_LOST:
    {
        TAL_PR_DEBUG("nwk_lost---");
        break;
    }
    case TAL_ZG_NWK_REMOTE_LEAVE:
    {
        TAL_PR_DEBUG("nwk_remote_leave---");
        app_light_ctrl_data_switch_set(TRUE);
 
        app_light_ctrl_proc();

        break;
    }
    case TAL_ZG_NWK_LOCAL_LEAVE:
    {
        TAL_PR_DEBUG("nwk_local_leave---");
        break;
    }
    case TAL_ZG_NWK_MF_TEST_LEAVE:
    {
        TAL_PR_DEBUG("nwk_mf_test_leave---");
        break;
    }
    default:
    {
        break;
    }
    }
}




STATIC VOID_T __led_flash_timer_cb(TIMER_ID timer_id, VOID_T *arg)
{
    sg_led_status = !sg_led_status;

    if (sg_led_status) {

        tal_gpio_write(LED_GPIO,TUYA_GPIO_LEVEL_LOW);

    } else {
        tal_gpio_write(LED_GPIO,TUYA_GPIO_LEVEL_HIGH);
    }

}

/**
* @brief Blinking LED when WiFi is in a wiring state
*
* @param[in] args: blink interval
* @return none
*/
STATIC VOID_T netled_status_task(VOID_T)
{

    app_gpio_init();

    tal_sw_timer_create(__led_flash_timer_cb, NULL, &etimer_blink_sw);

    sg_led_status = 1;

    tal_sw_timer_start(etimer_blink_sw, 500, TAL_TIMER_CYCLE);

}



VOID_T tal_beacon_mf_test_callback(VOID_T)
{
    TAL_PR_DEBUG("enter beacon test\n");
    //TOD
    tal_sw_timer_start(etimer_mf_blink, 250, TAL_TIMER_ONCE);
}

/**
 * @brief reset factory default callback
 *
 * @param[in]   type: reset factory type
 * @return  VOID_T
 */
VOID_T tal_zg_reset_factory_default_callback(TAL_RESET_TYPE_T type)
{
    TAL_PR_DEBUG("receive reset to factory default cmd %d", type);
}

VOID_T tal_zll_target_commissioning_complete_callback(TAL_ZLL_TARGET_STATUS_E status)
{
    TAL_PR_DEBUG("zll target status %d",status);
}



VOID_T tal_time_sync_complete_callback(BOOL_T status, UINT_T time_sec)
{
    TIME_T v_time;
    v_time = tal_current_time_get();
    TAL_PR_DEBUG("-------utc time--------- %d",v_time);
    TAL_PR_DEBUG("sync time: %d",time_sec);
}



