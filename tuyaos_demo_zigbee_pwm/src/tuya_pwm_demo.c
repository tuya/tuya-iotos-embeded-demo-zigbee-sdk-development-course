/**
* @file app_light_control.c
* @brief Common process - adapter the flash api
* @version 0.1
* @date 2021-10-22
*
* @copyright Copyright 2020-2021 Tuya Inc. All Rights Reserved.
*
*/

#include "tal_watchdog.h"
#include "tuya_pwm_demo.h"
#include "tkl_pwm.h"
#include "tkl_network_manager.h"
#include "tal_system.h"
#include "tal_memory.h"
#include "tal_log.h"
#include "tal_pwm.h"
#include "tal_gpio.h"
#include "tal_uart.h"
#include "tal_flash.h"
#include "tal_sw_timer.h"
#include "tal_attribute_rw.h"

#define PWM_MIN_FREQ 100   ///< PWM MIN Frequency 100
#define PWM_MAX_FREQ 20000 ///< PWM MAX Frequency 20K
#define PWM_MAX_DUTY 1000  ///< PWM MAX Duty 1000 --> Precision 0.1%

#define pwm_ch_num 1
#define pwm_duty 500
#define pwm_frequency 1000
#define pwm_polarity TUYA_PWM_POSITIVE
#define PIN_NUM_ERROR (0xFF)
#define TUYA_FLASH_PAGE_SIZE (8 * 1024UL)

//STATIC light_ctrl_data_t sg_light_ctrl_data;
STATIC light_ctrl_handle_t sg_light_ctrl_handle_data;

STATIC ULONG_T sg_blink_time_ms = 0;
STATIC UCHAR_T sg_light_channel = 0;

STATIC USHORT_T sg_shade_off_time = 1000; ///< shade time ms
STATIC USHORT_T sg_shade_on_time = 1000;
STATIC UINT8_T sg_shade_flag = FALSE;

STATIC USHORT_T sg_usRemainTime100ms = 0;
STATIC ULONG_T sg_transtime_100ms = 0;
STATIC UINT8_T sg_level_move_down_to_off = FALSE;

TIMER_ID etimer_blink_sw;
TIMER_ID etimer_shande_param;

BOOL_T g_pwm_bPolarity = FALSE;
BOOL_T g_user_pwm_init_flag = FALSE;

extern OPERATE_RET tkl_pwm_mapping_to_gpio(UINT32_T ch_id, UINT32_T gpio_id);

/**
 * @brief
 *
 * @param str
 * @return VOID_T
 */
VOID_T dev_uart_output(IN CONST CHAR_T *str)
{
    tal_uart_write(USER_UART0, str, strlen(str));
}

/**
 * @brief
 *
 * @return OPERATE_RET
 */
OPERATE_RET app_pwm_init(VOID_T)
{
    OPERATE_RET v_ret = OPRT_COM_ERROR;

    TUYA_PWM_BASE_CFG_T v_cfg = {
        .duty = 0,
        .frequency = pwm_frequency,
        .polarity = pwm_polarity,
    };

    tkl_pwm_mapping_to_gpio(1, 14);//PC5

    v_ret = tal_pwm_init(1, &v_cfg);

    v_ret = tal_pwm_start(1);
    if(v_ret!=OPRT_OK) {
        TAL_PR_DEBUG("pwm init error!");
    }
    g_user_pwm_init_flag = TRUE;
    g_pwm_bPolarity = pwm_polarity;
    app_light_ctrl_ch_num_set(pwm_ch_num);
    TAL_PR_DEBUG("pwm init ok!");
    return v_ret;
}

/**
 * @berief: pwm soc set positive duty
 * @param {ULONG_T duty --> pwm send duty}
 * @param {UCHAR_T channel --> pwm send out channel}
 * @attention channel --> according to the initlize order
 * @return: none
 */
STATIC VOID_T __app_ctrl_pwm_duty_set(UCHAR_T channel, USHORT_T duty)
{
    //TAL_PR_DEBUG("channel = %d , duty = %d\n", channel, duty);
    if (duty > PWM_MAX_DUTY)
    {
        TAL_PR_DEBUG("PWM SET DUTY ERR");
        return;
    }
    if (channel < sg_light_channel)
    {
        TAL_PR_DEBUG("PWM SET DUTY CHANNEL ERR");
        return;
    }

    TUYA_PWM_BASE_CFG_T v_dcb = {
        .duty = duty,
        .frequency = 1000,
        .polarity = TUYA_PWM_NEGATIVE,
    };

    if (OPRT_OK != tal_pwm_info_set(channel, &v_dcb))
    {
        TAL_PR_DEBUG("PWM SET DUTY ERR");
    }
}

/**
 * @note: light send control data
 * @param {usl_Value} range 0~1000
 * @return: OPERATE_RET
 */
STATIC OPERATE_RET __app_light_ctrl_rgbcw_set(USHORT_T l_value)
{
    if (g_user_pwm_init_flag != TRUE)
    {
        TAL_PR_DEBUG("user pwm not init!");
        return OPRT_INVALID_PARM;
    }
    if (l_value > PWM_MAX_DUTY)
    {
        TAL_PR_DEBUG("USER PWM data is invalid!");
        return OPRT_INVALID_PARM;
    }
    if (g_pwm_bPolarity==TUYA_PWM_NEGATIVE)
    {
        l_value = PWM_MAX_DUTY - l_value;
    }
    switch (sg_light_channel)
    {
    case 1:                                  /* 1 way -- CW */
        __app_ctrl_pwm_duty_set(1, l_value); /* send l value */
        break;
    default:
        TAL_PR_DEBUG("pwm drive channel set error!!");
        break;
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
    tal_sw_timer_stop(etimer_shande_param);
    TAL_PR_DEBUG("Shade STOP >>>  RGBCW : %d", sg_light_ctrl_handle_data.current_val.warm);
}

/**
 * @note: light ctrl blink proc
 * @param {none}
 * @return: UCHAR_T
 */
STATIC VOID_T __app_light_ctrl_blink_display(VOID_T)
{
    OPERATE_RET ret = -1;
    STATIC UCHAR_T cnt = 0;
    light_data_t ctrl_data;

    tal_system_memset(&ctrl_data, 0, SIZEOF(light_data_t));
    ctrl_data.white = 0;
    ctrl_data.warm = (cnt % 2) ? 1000 : 0;
    ret = __app_light_ctrl_rgbcw_set(ctrl_data.warm);
    TAL_PR_DEBUG("blink...W->%d, ctrl_data.warm");
    TAL_PR_DEBUG("blink...C->%d, W->%d", ctrl_data.white, ctrl_data.warm);
    if (ret != OPRT_OK)
    {
        TAL_PR_DEBUG("blink set RGBCW error!");
    }
    cnt++;
}


/**
 * @note: light ctrl blink timer callback
 * @param {none}
 * @return: none
 */
VOID_T app_light_ctrl_blink_timer_cb(TIMER_ID timer_id, VOID_T *arg)
{
    __app_light_ctrl_blink_display();
    tal_sw_timer_start(etimer_blink_sw, sg_blink_time_ms, TAL_TIMER_ONCE);
}
/**
 * @description: start blink
 * @param {IN ULONG_T blink_time -> blink phase}
 * @attention: blink display will as the parm
 *             -- NetColor, usNetBright in configuration.
 * @return: none
 */
OPERATE_RET app_light_ctrl_blink_start(ULONG_T blink_time)
{
    sg_blink_time_ms = 1000;
    //__app_light_ctrl_stop();
    tal_sw_timer_start(etimer_blink_sw, 10, TAL_TIMER_ONCE);

    return OPRT_OK;
}

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

    return OPRT_OK;
}
/**
 * @brief Set the pwm channel num object
 * 
 * @param channel_num 
 */
VOID_T app_light_ctrl_ch_num_set(UCHAR_T channel_num)
{
    sg_light_channel = channel_num;
}

