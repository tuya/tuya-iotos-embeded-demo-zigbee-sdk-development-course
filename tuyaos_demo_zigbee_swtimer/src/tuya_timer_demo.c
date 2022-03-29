/*
 * @FileName: your project
 * @Author: Tuya
 * @Email:
 * @LastEditors: Tuya
 * @Date: 2022-03-07 14:24:24
 * @LastEditTime: 2022-03-28 09:56:27
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company:  http://www.tuya.com
 * @Description:
 */

#include "tal_system.h"
#include "tal_gpio.h"
#include "tuya_timer_demo.h"
#include "tal_adc.h"
#include "tkl_adc.h"
#include "tal_uart.h"
#include "tal_log.h"
#include "tal_sw_timer.h"
#include "tal_time_sync.h"
#include "tal_memory.h"

/***********************************************************
************************micro define************************
***********************************************************/

#define PIN_NUM_ERROR (0xFF)
#define TUYA_FLASH_PAGE_SIZE (8 * 1024UL)
#define led_on TUYA_GPIO_LEVEL_LOW
#define led_off TUYA_GPIO_LEVEL_HIGH

STATIC led_data_t sg_led_data;
STATIC ULONG_T sg_blink_time_ms = 0;
STATIC UINT8_T led_status = led_off;

TIMER_ID etimer_blink_sw;

BOOL_T g_user_gpio_init_flag = FALSE;

/**
 * @brief
 *
 * @return OPERATE_RET
 */
OPERATE_RET tuya_gpio_init(VOID_T)
{
    OPERATE_RET v_ret = OPRT_COM_ERROR;

    TUYA_GPIO_BASE_CFG_T v_cfg = {
        .mode = TUYA_GPIO_PUSH_PULL,
        .direct = TUYA_GPIO_OUTPUT,
        .level = TUYA_GPIO_LEVEL_HIGH,
    };

    v_ret = tal_gpio_init(14, &v_cfg);

    if  (v_ret != OPRT_OK) {
        TAL_PR_DEBUG("gpio init error!");
    }

    g_user_gpio_init_flag = TRUE;

    TAL_PR_DEBUG("gpio init ok!");
    return v_ret;
}

/**
 * @note: Led control proc
 * @param {none}
 * @return: OPERATE_RET
 */
OPERATE_RET led_ctrl_proc(VOID_T)
{
    led_status = !led_status;

    if (led_status) {

        tal_gpio_write(14,TUYA_GPIO_LEVEL_HIGH);

    } else {
        tal_gpio_write(14,TUYA_GPIO_LEVEL_LOW);
    }

    TAL_PR_DEBUG("led status is: %d", led_status);

}


/**
 * @note: led ctrl blink timer callback
 * @param {none}
 * @return: none
 */
VOID_T led_ctrl_blink_timer_cb(TIMER_ID timer_id, VOID_T *arg)
{
    led_ctrl_proc();
    tal_sw_timer_start(etimer_blink_sw, 1000, TAL_TIMER_ONCE);

}
/**
 * @description: start blink
 * @param {IN ULONG_T blink_time -> blink phase}
 * @attention: blink display will as the parm
 *             -- NetColor, usNetBright in configuration.
 * @return: none
 */
OPERATE_RET led_ctrl_blink_start(ULONG_T blink_time)
{
    sg_blink_time_ms = 1000;
    tal_sw_timer_start(etimer_blink_sw, 10, TAL_TIMER_ONCE);

    return OPRT_OK;
}


