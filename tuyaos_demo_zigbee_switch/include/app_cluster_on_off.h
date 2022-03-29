/*
 * @FileName: your project
 * @Author: Tuya
 * @Email:
 * @LastEditors: Tuya
 * @Date: 2022-02-18 14:36:43
 * @LastEditTime: 2022-03-22 17:48:57
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company:  http://www.tuya.com
 * @Description:
 */
#ifndef __ON_OFF_CLUSTER_H__
#define __ON_OFF_CLUSTER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "tuya_cloud_types.h"
#include "app_common.h"
#include "tal_data_send.h"
#include "tkl_endpoint_register.h"


  typedef struct
  {
    UINT8_T onoff_status; /* on off setting */
    ULONG_T count_down_cnt;
  } light_ctrl_data_t;



  OPERATE_RET app_light_ctrl_blink_stop(VOID_T);
  //OPERATE_RET app_light_ctrl_blink_start(ULONG_T blink_time);
  OPERATE_RET app_light_ctrl_data_switch_set(UINT8_T on_off);
  OPERATE_RET app_light_ctrl_data_switch_get(UINT8_T *on_off);

  OPERATE_RET app_onoff_cluster_handler(UCHAR_T cmd, UINT8_T *payload, UINT8_T payload_len, ZIGBEE_CMD_T cmd_type);
    /**
 * @brief
 *
 * @param cmd
 * @param payload
 * @param payload_len
 * @param cmd_type
 * @return OPERATE_RET
 */
  OPERATE_RET app_level_cluster_handler(UCHAR_T cmd, UINT8_T *payload, UINT8_T payload_len, ZIGBEE_CMD_T cmd_type);

  /**
 * @note app_onoff_count_down_response
 * @param [in] {ULONG_T} set count down s
 * @return: none
 */
  VOID_T app_onoff_count_down_response(ULONG_T remain_time_sec);

  /**
 * @note app_onoff_handle_count_down
 * @param [in]{UCHAR_T} delay_ms 
 * @param 
 * @return: none
 */
  VOID_T app_onoff_handle_count_down(ULONG_T delay_ms);

  VOID_T app_onoff_set_count_down_report_flag_time(UINT8_T flag, ULONG_T delay_ms);

  /**
 * @note report onoff attribute value
 * @param [in] {TAL_SEND_QOS_E} Qos
 * @param [in] {USHORT_T} delay_ms
 * @param [in] {UINT8_T} onoff
 * @return: none
 */
  VOID_T app_onoff_report_value(TAL_SEND_QOS_E qos, USHORT_T delay_ms, UINT8_T onoff);

  /**
 * @note app_onoff_report_count_down_data
 * @param [in] {TAL_SEND_QOS_E} QOS 
 * @param [in] {USHORT_T} delay_ms
 * @param [in] {USHORT_T} countdown
 * @return: none
 */
  VOID_T app_onoff_report_count_down_data(TAL_SEND_QOS_E Qos, USHORT_T delay_ms, ULONG_T countdown);

VOID_T app_light_ctrl_save_data_delay_cb(TIMER_ID timer_id, VOID_T *arg);

VOID_T app_light_ctrl_power_on_sync_cb(TIMER_ID timer_id, VOID_T *arg);

VOID_T app_light_ctrl_join_end_sync_cb(TIMER_ID timer_id, VOID_T *arg);

  /**
 * @note: return app_light_count_down_cnt_get
 * @param [in] none
 * @return: app_light_count_down_cnt_get
 */
ULONG_T app_light_count_down_cnt_get(VOID_T);

  /**
 * @note: set light count down cnt
 * @param [in] {ULONG_T}count
 * @return: none
 */
VOID_T app_light_count_down_cnt_set(ULONG_T count);

#ifdef __cplusplus
  extern "C"
  {
#endif
#endif