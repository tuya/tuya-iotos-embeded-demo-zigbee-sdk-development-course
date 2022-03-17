/**
* @file app_cluster_on_off.c
* @brief Common process - adapter the flash api
* @version 0.1
* @date 2021-10-22
*
* @copyright Copyright 2020-2021 Tuya Inc. All Rights Reserved.
*
*/
#include "tal_log.h"
#include "tal_gpio.h"
#include "tal_memory.h"
#include "tal_system.h"
#include "tal_sw_timer.h"
#include "tal_attribute_rw.h"
#include "app_cluster_on_off.h"

#define TY_CMD_SET_COUNT_DOWN_CMD_ID 0xF0 ///<   tuya private cmd
#define LED_MODE_ATTRIBUTE_ID         0x8001
#define COUNT_DOWN_MAX 86400

STATIC UCHAR_T count_down_report_onoff_flag = FALSE;
STATIC ULONG_T count_down_report_delay = 0;
extern TIMER_ID etimer_countdown;


extern TIMER_ID etimer_blink_sw;

extern TIMER_ID etimer_join_end_sync;
extern TIMER_ID etimer_save_data_delay;

STATIC light_ctrl_data_t sg_light_ctrl_data;


//STATIC ULONG_T sg_blink_time_ms = 0;

STATIC USHORT_T sg_usRemainTime100ms = 0;
STATIC UINT8_T sg_shade_flag = FALSE;


/**
 * @note app on/off command handler
 * @param [in]{UCHAR_T} onoff
 * @param [in]{ZIGBEE_CMD_T} single or group transmit cmd
 * @return: none
 */
STATIC VOID_T __app_onoff_cmd_set_value_handler(ZIGBEE_CMD_T cmd_type, UCHAR_T onoff)
{
    TAL_SEND_QOS_E QOS = QOS_1;
    ULONG_T delay_ms = 0;
    OPERATE_RET ret;
    UINT8_T i = 0;
    TAL_PR_DEBUG("Receive on/off: %d", onoff);

    if (cmd_type == ZIGBEE_CMD_GROUP)
    {
        delay_ms = 10000 + tal_system_get_random(1000);
        QOS = QOS_0;
    }
    app_onoff_report_value(QOS, delay_ms, onoff);
    //dont report onoff again in the count down handler
    count_down_report_onoff_flag = 0;
    count_down_report_delay = delay_ms;
    app_onoff_data_count_down_set(0);

    ret = app_light_ctrl_data_switch_set(onoff);
    if (ret != OPRT_OK)
    {
        return;
    }
    app_light_ctrl_proc();
    //update on/off attribute
    tal_zg_write_attribute(1,
                           CLUSTER_ON_OFF_CLUSTER_ID,
                           ATTR_ON_OFF_ATTRIBUTE_ID,
                           &onoff,
                           ATTR_BOOLEAN_ATTRIBUTE_TYPE);
}

/**
 * @brief
 *
 * @param Qos
 * @param delay_ms
 * @param onoff
 * @return VOID_T
 */
VOID_T app_onoff_report_value(TAL_SEND_QOS_E Qos, USHORT_T delay_ms, UINT8_T onoff)
{
    TAL_ZG_SEND_DATA_T send_data;

    tal_system_memset(&send_data, 0, sizeof(TAL_ZG_SEND_DATA_T));

    send_data.delay_time = delay_ms;
    send_data.random_time = 0;
    send_data.zcl_id = ZCL_ID_ONOFF;
    send_data.qos = Qos;
    send_data.direction = ZG_ZCL_DATA_SERVER_TO_CLIENT;
    send_data.frame_type = ZG_ZCL_FRAME_TYPE_GLOBAL;
    send_data.command_id = CMD_REPORT_ATTRIBUTES_COMMAND_ID;

    // unicast to gateway
    send_data.addr.mode = SEND_MODE_DEV;
    send_data.addr.type.dev.dst_addr = TUYA_GATEWAY_ADDRESS;
    send_data.addr.type.dev.dst_ep = 0x01;
    send_data.addr.type.dev.src_ep = TUYA_PRIMARY_ENDPOINT;
    send_data.addr.type.dev.cluster_id = CLUSTER_ON_OFF_CLUSTER_ID;

    send_data.data.zg.attr_sum = 1;
    send_data.data.zg.attr[0].attr_id = ATTR_ON_OFF_ATTRIBUTE_ID;
    send_data.data.zg.attr[0].type = ATTR_BOOLEAN_ATTRIBUTE_TYPE;
    send_data.data.zg.attr[0].size = 1;
    send_data.data.zg.attr[0].value[0] = onoff;
    tal_zg_send_data(&send_data, NULL, 2000);
    TAL_PR_DEBUG("Report onoff qos:%d VALUE %d", Qos, onoff);
}
/**
 * @note app_onoff_report_count_down_data
 * @param [in] {TAL_SEND_QOS_E} QOS 
 * @param [in] {USHORT_T} delay_ms
 * @param [in] {USHORT_T} countdown
 * @return: none
 */
VOID_T app_onoff_report_count_down_data(TAL_SEND_QOS_E Qos, USHORT_T delay_ms, ULONG_T countdown)
{
    TAL_ZG_SEND_DATA_T send_data;
    tal_system_memset(&send_data, 0, sizeof(TAL_ZG_SEND_DATA_T));

    send_data.delay_time = delay_ms;
    send_data.qos = Qos;
    send_data.zcl_id = ZCL_ID_COUNT_DOWN;
    send_data.direction = ZG_ZCL_DATA_SERVER_TO_CLIENT;
    send_data.command_id = CMD_REPORT_ATTRIBUTES_COMMAND_ID;
    send_data.addr.mode = SEND_MODE_DEV;
    send_data.addr.type.dev.cluster_id = CLUSTER_ON_OFF_CLUSTER_ID;
    send_data.addr.type.dev.src_ep = 1;
    send_data.addr.type.dev.dst_ep = 1;
    send_data.data.zg.attr_sum = 1;
    send_data.data.zg.attr[0].attr_id = TY_ATTR_COUNT_DOWN_ATTR_ID;
    send_data.data.zg.attr[0].type = ATTR_INT32U_ATTRIBUTE_TYPE;
    send_data.data.zg.attr[0].size = 4;
    send_data.data.zg.attr[0].value[0] = countdown;
    send_data.data.zg.attr[0].value[1] = countdown >> 8;
    send_data.data.zg.attr[0].value[2] = countdown >> 16;
    send_data.data.zg.attr[0].value[3] = countdown >> 24;

    //tal_zg_clear_send_data(ZG_CLEAR_ALL_ZCL_ID, &send_data.zcl_id);
    tal_zg_send_data(&send_data, NULL, 1000);
    TAL_PR_DEBUG("Report count down: %d", countdown);
}
/**
 * @note app_onoff_count_down_response
 * @param [in] {ULONG_T} remain_time_sec
 * @return: none
 */
VOID_T app_onoff_count_down_response(ULONG_T remain_time_sec)
{
    STATIC ULONG_T last_remain_sec = 0;
    UINT8_T onoff = 0;

    TAL_PR_DEBUG("%s", __FUNCTION__);
    if (last_remain_sec == remain_time_sec)
    {
        TAL_PR_DEBUG("last remian is equal to reamin!");
    }
    else
    {
        app_onoff_report_count_down_data(QOS_1, count_down_report_delay, (ULONG_T)remain_time_sec);
    }

    if (remain_time_sec == 0)
    {
        app_light_ctrl_data_switch_get(&onoff);
        TAL_PR_DEBUG("count_down_report_onoff_flag= %d", count_down_report_onoff_flag);
        if (count_down_report_onoff_flag == TRUE)
        {
            TAL_PR_DEBUG("count_down_report_delay %d", count_down_report_delay);
            app_onoff_report_value(QOS_0, count_down_report_delay, onoff);
            app_onoff_set_count_down_report_flag_time(FALSE, 1000);
            TAL_PR_DEBUG("report onoff %d", onoff);
        }
        tal_zg_write_attribute(1,
                               CLUSTER_ON_OFF_CLUSTER_ID,
                               ATTR_ON_OFF_ATTRIBUTE_ID,
                               &onoff,
                               ATTR_BOOLEAN_ATTRIBUTE_TYPE);
    }
    last_remain_sec = remain_time_sec;
}




/**
 * @note: set light countdown value
 * @param {IN ULONG_T count_down_cnt -> unit:second}
 * @attention: countdown lave time will return with
 *              calling app_onoff_count_down_response function every minutes.
 *              switch status will return with calling
 *              light_ctrl_data_switch_response function when countdown active.
 * @return: OPERATE_RET -> OPRT_OK set countdown OK.
 */
OPERATE_RET app_onoff_data_count_down_set(ULONG_T count_down_cnt)
{
    if (count_down_cnt > COUNT_DOWN_MAX)
    {
        TAL_PR_DEBUG("Set count down value error!");
        return OPRT_INVALID_PARM;
    }
    //app_light_count_down_cnt_set(count_down_cnt);
    if (count_down_cnt <= 0)
    {
        //cancel coutdown proc
        tal_sw_timer_stop(etimer_countdown);
        app_onoff_count_down_response(0);
        return OPRT_OK;
    }
    else
    {
        tal_sw_timer_start(etimer_countdown, 1000, TAL_TIMER_ONCE);
    }

    //upload countdown value
    app_onoff_count_down_response(count_down_cnt);

    return OPRT_OK;
}
/**
 * @note app_onoff_handle_count_down
 * @param [in]{UCHAR_T} onoff
 * @param [in]{ZIGBEE_CMD_T} cmd_type
 * @return: none
 */
VOID_T app_onoff_handle_count_down(ULONG_T delay_ms)
{
    UINT8_T pre_onoff;
    TAL_PR_DEBUG("%s", __FUNCTION__);
    app_light_ctrl_data_switch_get(&pre_onoff);
    app_light_ctrl_data_switch_set(1);
    if (pre_onoff != 1)
    {
        app_onoff_report_value(QOS_0, delay_ms, 1);
        count_down_report_onoff_flag = 0;
        count_down_report_delay = delay_ms;
        app_onoff_data_count_down_set(0);
    }
}
/**
 * @brief 
 * 
 * @param flag 
 * @param delay_ms 
 */
VOID_T app_onoff_set_count_down_report_flag_time(UINT8_T flag, ULONG_T delay_ms)
{
    count_down_report_onoff_flag = flag;
    count_down_report_delay = delay_ms;
}
/**
 * @brief handle zcl on/off cluster command
 * 
 * @param cmd 
 * @param payload 
 * @param payload_len 
 * @param cmd_type 
 * @return OPERATE_RET 
 */
OPERATE_RET app_onoff_cluster_handler(UCHAR_T cmd, UINT8_T *payload, UINT8_T payload_len, ZIGBEE_CMD_T cmd_type)
{
    UINT8_T onoff_value;
    ULONG_T count_down_data = 0;
    switch (cmd)
    {
    case CMD_OFF_COMMAND_ID:
    {
        __app_onoff_cmd_set_value_handler(cmd_type, FALSE);
        break;
    }
    case CMD_ON_COMMAND_ID:
    {
        __app_onoff_cmd_set_value_handler(cmd_type, TRUE);
        break;
    }
    case CMD_TOGGLE_COMMAND_ID:
    {
        app_light_ctrl_data_switch_get(&onoff_value);
        __app_onoff_cmd_set_value_handler(cmd_type, !onoff_value);
        break;
    }
    case TY_CMD_SET_COUNT_DOWN_CMD_ID:
    {
        //tuya Countdown
        count_down_data = (ULONG_T)payload[0] + ((ULONG_T)payload[1] << 8) + ((ULONG_T)payload[2] << 16) + ((ULONG_T)payload[3] << 24);
        app_onoff_set_count_down_report_flag_time(TRUE, 1000);
        app_onoff_data_count_down_set(count_down_data);
        break;
    }
    default:
        break;
    }
    return OPRT_OK;
}



/**
 * @brief
 *
 * @param cmd
 * @param payload
 * @param payload_len
 * @param cmd_type
 * @return OPERATE_RET
 */
OPERATE_RET app_level_cluster_handler(UCHAR_T cmd, UINT8_T *payload, UINT8_T payload_len, ZIGBEE_CMD_T cmd_type)
{
    UINT8_T onoff = FALSE;
    app_light_ctrl_data_switch_get(&onoff);
    switch (cmd)
    {
    case CMD_MOVE_TO_LEVEL_COMMAND_ID:
    case CMD_MOVE_TO_LEVEL_WITH_ON_OFF_COMMAND_ID:
    {
        UCHAR_T level, option_mask = 0, option_override = 0, option;
        USHORT_T trans_time;
        level = payload[0];
        trans_time = payload[1] + (payload[2] << 8);

        if (payload_len == 5)
        {
            option_mask = payload[payload_len - 2];
            option_override = payload[payload_len - 1];
        }
        option = app_zcl_get_option_value(CLUSTER_LEVEL_CONTROL_CLUSTER_ID, option_mask, option_override);
        if (cmd == CMD_MOVE_TO_LEVEL_WITH_ON_OFF_COMMAND_ID)
        {
           //__app_level_z3_cmd_move_to_level_handler(cmd_type, level, trans_time, TRUE);
        }
        else
        {
            if (1 == onoff || option == 0x01)
            {
                //__app_level_z3_cmd_move_to_level_handler(cmd_type, level, trans_time, FALSE);
            }
        }
        break;
    }
 
    case CMD_STEP_COMMAND_ID: //   don't effect switch
    case CMD_STEP_WITH_ON_OFF_COMMAND_ID:
    {
        //  effect switch
        UCHAR_T option_mask = 0, option_override = 0, option;
        UCHAR_T step_mode, step_size;
        USHORT_T trans_time;

        step_mode = payload[0];
        step_size = payload[1];
        trans_time = payload[2] + (payload[3] << 8);

        if (payload_len == 6)
        {
            option_mask = payload[payload_len - 2];
            option_override = payload[payload_len - 1];
        }
        option = app_zcl_get_option_value(CLUSTER_LEVEL_CONTROL_CLUSTER_ID, option_mask, option_override);
        if (cmd == CMD_STEP_WITH_ON_OFF_COMMAND_ID)
        {
           //__app_level_cmd_step_level_handler(step_mode, step_size, trans_time, TRUE);
        }
        else
        {
            if (onoff == 1 || option == 0x01)
            {
                //__app_level_cmd_step_level_handler(step_mode, step_size, trans_time, FALSE);
            }
        }
        break;
    }

    case CMD_STOP_COMMAND_ID:
    case CMD_STOP_WITH_ON_OFF_COMMAND_ID:
    case CMD_MOVE_COMMAND_ID:
    case CMD_MOVE_WITH_ON_OFF_COMMAND_ID:
    {
        //move mode + rate
        UCHAR_T move_mode, option_mask = 0, option_override = 0, option;
        UCHAR_T move_rate;

        move_mode = payload[0];
        move_rate = payload[1];

        if (payload_len == 4)
        {
            option_mask = payload[payload_len - 2];
            option_override = payload[payload_len - 1];
        }
        option = app_zcl_get_option_value(CLUSTER_LEVEL_CONTROL_CLUSTER_ID, option_mask, option_override);
        if (cmd == CMD_MOVE_WITH_ON_OFF_COMMAND_ID)
        {
           // __app_level_cmd_move_level_handler(move_mode, move_rate, TRUE);
        }
        else
        {
            if (1 == onoff || option == 0x01)
            {
                // light on, need to handler
               // __app_level_cmd_move_level_handler(move_mode, move_rate, FALSE);
            }
        }

        break;
    }
    default:
        break;
    }
    return OPRT_OK;
}



/**
 * @note: get light switch data
 * @param {OUT UINT8_T *onff -> switch data return}
 * @return: OPERATE_RET
 */
OPERATE_RET app_light_ctrl_data_switch_get(UINT8_T *on_off)
{
    *on_off = sg_light_ctrl_data.onoff_status;

    return OPRT_OK;
}


OPERATE_RET app_light_ctrl_data_switch_set(UINT8_T on_off)
{
    UINT8_T last_status;

    last_status = sg_light_ctrl_data.onoff_status;

    if (TRUE == on_off)
    {
        sg_light_ctrl_data.onoff_status = TRUE;
    }
    else
    {
        sg_light_ctrl_data.onoff_status = FALSE;
    }
    if (on_off == last_status)
    {
        TAL_PR_DEBUG("the same switch set");
        return OPRT_INVALID_PARM;
    }

    return OPRT_OK;
}

/**
 * @brief 
 * 
 */
STATIC VOID_T __app_light_ctrl_stop(VOID_T)
{
    sg_usRemainTime100ms = 0;
    sg_shade_flag = FALSE;

    tal_sw_timer_stop(etimer_blink_sw);

}


STATIC VOID_T __app_light_ctrl_normal_display(VOID_T)
{
    app_data_t ctrl_data;

     tal_system_memset(&ctrl_data, 0, SIZEOF(app_data_t));

    __app_light_ctrl_stop();

    app_light_ctrl_data_switch_set(1);

    tal_sw_timer_start(etimer_join_end_sync, 10000, TAL_TIMER_ONCE);
    tal_sw_timer_start(etimer_save_data_delay, 10, TAL_TIMER_ONCE);
}


STATIC UINT8_T __app_light_ctrl_write_app_data_attr(app_all_data_t *app_data)
{
    UCHAR_T buffer[sizeof(app_all_data_t) + 1] = {0};
    if (app_data == NULL)
    {
        TAL_PR_DEBUG("app_data ERROR");
        return FALSE;
    }
    buffer[0] = sizeof(app_all_data_t);
    tal_system_memcpy(&buffer[1], app_data, sizeof(app_all_data_t));
    OPERATE_RET ret = tal_zg_write_attribute(1,
                                             CLUSTER_ON_OFF_CLUSTER_ID,
                                             TY_ATTR_COUNT_DOWN_ATTR_ID,
                                             buffer,
                                             ATTR_ARRAY_ATTRIBUTE_TYPE);
    if (ret != OPRT_OK)
    {
        TAL_PR_DEBUG("Write app_data ERROR:%d", ret);
        return FALSE;
    }
    return TRUE;
}


/**
 * @description: start blink
 * @param {IN ULONG_T blink_time -> blink phase}
 * @attention: blink display will as the parm
 *             -- NetColor, usNetBright in configuration.
 * @return: none
 */
// OPERATE_RET app_light_ctrl_blink_start(ULONG_T blink_time)
// {
//     sg_blink_time_ms = 1000;
//     __app_light_ctrl_stop();

//     tal_sw_timer_start(etimer_blink_sw, 10, TAL_TIMER_ONCE);

//     return OPRT_OK;
// }

/**
 * @description: stop blink
 * @param {type} none
 * @attention: blink stop will directly go to normal status display
 *              normal status will bright as default bright parm
 *              -- usDefBright,usDefTemper,DefColor in configuration.
 * @return: none
 */
OPERATE_RET app_light_ctrl_blink_stop(VOID_T)
{
    tal_sw_timer_stop(etimer_blink_sw);

    __app_light_ctrl_normal_display();


    return OPRT_OK;
}

/**
 * @note: Light control proc
 * @param {none}
 * @return: OPERATE_RET
 */
OPERATE_RET app_light_ctrl_proc(VOID_T)
{
    STATIC UINT8_T last_switch_status = FALSE;

    TAL_PR_DEBUG("sg_lighht_ctrl_data.onoff = %d, last_switch_status = %d",sg_light_ctrl_data.onoff_status, last_switch_status);

    if (sg_light_ctrl_data.onoff_status == TRUE)
    {
        ///< OFF
        tal_gpio_write(LED_GPIO, TUYA_GPIO_LEVEL_HIGH);

    }
    else
    {
        ///< ON
        if (last_switch_status == FALSE)
        {
            ///< light is off

        }
        tal_gpio_write(LED_GPIO, TUYA_GPIO_LEVEL_LOW);
    }
    last_switch_status = sg_light_ctrl_data.onoff_status;

    return OPRT_OK;
}


/**
 * @brief
 *
 * @param timer_id
 * @param arg
 * @return STATIC
 */
VOID_T app_light_ctrl_save_data_delay_cb(TIMER_ID timer_id, VOID_T *arg)
{
    TAL_PR_DEBUG("Run Into>>>%s", __FUNCTION__);
    app_data_t v_app_dcb = {
        .onoff = sg_light_ctrl_data.onoff_status,
    };

    tal_flash_erase(FLASH_ADDR_APP_DATA, MG21_FLASH_PAGE_SIZE);
    tal_flash_write(FLASH_ADDR_APP_DATA, (UCHAR_T *)&v_app_dcb, sizeof(app_data_t));

    app_all_data_t v_dcb = {
        .onoff = sg_light_ctrl_data.onoff_status,

    };
    __app_light_ctrl_write_app_data_attr(&v_dcb);
}


/**
 * @brief
 *
 * @param timer_id
 * @param arg
 * @return VOID_T
 */
VOID_T app_light_ctrl_power_on_sync_cb(TIMER_ID timer_id, VOID_T *arg)
{

    TAL_PR_DEBUG("Power On Sync Light State.");

    app_light_ctrl_data_switch_get((UINT8_T *)&g_power_on_state);

    app_onoff_report_value(QOS_VIP_0, 0, g_power_on_state);

    app_onoff_report_count_down_data(QOS_1, 800, 0);
    //update on/off attribute
    tal_zg_write_attribute(1,
                           CLUSTER_ON_OFF_CLUSTER_ID,
                           ATTR_ON_OFF_ATTRIBUTE_ID,
                           &g_power_on_state,
                           ATTR_BOOLEAN_ATTRIBUTE_TYPE);
}


/**
 * @brief
 *
 * @param timer_id
 * @param arg
 * @return VOID_T
 */
VOID_T app_light_ctrl_join_end_sync_cb(TIMER_ID timer_id, VOID_T *arg)
{

    TAL_PR_DEBUG("join end Sync Light State.");

    app_onoff_report_value(QOS_1, 0, sg_light_ctrl_data.onoff_status); ///<  report data need

    tal_zg_write_attribute(1,
                           CLUSTER_ON_OFF_CLUSTER_ID,
                           ATTR_ON_OFF_ATTRIBUTE_ID,
                           &sg_light_ctrl_data.onoff_status,
                           ATTR_BOOLEAN_ATTRIBUTE_TYPE);
}
