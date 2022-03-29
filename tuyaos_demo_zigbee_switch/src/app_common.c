/*
 * @FileName: your project
 * @Author: Tuya
 * @Email: 
 * @LastEditors: Tuya
 * @Date: 2022-02-18 14:29:54
 * @LastEditTime: 2022-03-28 09:54:32
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company:  http://www.tuya.com
 * @Description: 
 */
/*************************************************************************************/
/* Automatically-generated file. Do not edit! */
/*************************************************************************************/

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
#include "app_common.h"

#define KEY_SCAN_MAX_CNT (250)
#define PIN_NUM_ERROR (0xFF)
#define TUYA_FLASH_PAGE_SIZE (8 * 1024UL)

#define gpio_ch_num 1

extern TIMER_ID etimer_shande;
extern TIMER_ID etimer_key_scan;
extern TIMER_ID etimer_blink_sw;
extern TIMER_ID etimer_countdown;
extern TIMER_ID etimer_join_start_delay;
extern TIMER_ID etimer_power_on_sync;
extern TIMER_ID etimer_join_end_sync;

STATIC UINT16_T sg_key_scan_cnt = 0;
STATIC UINT8_T sg_led_status = 0;

BOOL_T g_user_gpio_init_flag = FALSE;

UINT8_T g_power_on_state = 0;
UINT16_T g_power_on_bright = 0;
UINT8_T g_relay_onoff_status[gpio_ch_num + 1] = {0};

/**
 * @note __dev_power_on_reset_data
 * @param [in] none 
 * @param [out] none
 * @note user can recover light date or set default data in this function
 * @return: none
 */
STATIC VOID_T __dev_power_on_reset_data(VOID_T)
{
    ZG_NWK_TYPE_E nwk_type = tkl_zg_nwk_type_get();

    app_data_t v_app_dcb;

    tal_flash_read(FLASH_ADDR_APP_DATA, (UCHAR_T *)&v_app_dcb, sizeof(app_data_t));

    app_light_ctrl_data_switch_set(v_app_dcb.onoff);
    TAL_PR_DEBUG("nwk_type =%d", nwk_type);

    ULONG_T delay_ms = 0;
    delay_ms = 15000 + tal_system_get_random(5000);
    tal_sw_timer_start(etimer_power_on_sync, delay_ms, TAL_TIMER_ONCE);
}

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
 * @param args 
 * @return VOID_T 
 */
STATIC VOID_T __dev_pin_irq_handle(VOID_T *args)
{
    sg_key_scan_cnt = 0;
    tal_sw_timer_start(etimer_key_scan, 20, TAL_TIMER_ONCE);
}

/**
 * @brief
 *
 * @return OPERATE_RET
 */
OPERATE_RET app_gpio_init(VOID_T)
{
    OPERATE_RET v_ret = OPRT_COM_ERROR;

    TUYA_GPIO_BASE_CFG_T v_cfg = {
        .mode = TUYA_GPIO_PUSH_PULL,
        .direct = TUYA_GPIO_OUTPUT,
        .level = TUYA_GPIO_LEVEL_HIGH,
    };

    v_ret = tal_gpio_init(LED_GPIO, &v_cfg);

    if  (v_ret!=OPRT_OK) {
        TAL_PR_DEBUG("gpio init error!");
    }

    g_user_gpio_init_flag = TRUE;

    TAL_PR_DEBUG("gpio init ok!");
    return v_ret;
}

/**
 * @note light init
 *          1. gpio init;
 *          2. reset cnt processing
 *          3. power on config default light dada
 *          4. turn on light
 * @param [in] none
 * @return: OPERATE_RET
 */
OPERATE_RET app_light_init(VOID_T)
{

    app_gpio_init();
    __dev_power_on_reset_data();
    app_light_ctrl_proc();

    return OPRT_OK;
}

/**
 * @brief
 *
 * @return VOID_T
 */
VOID_T app_button_init(VOID_T)
{
    TUYA_GPIO_IRQ_T irq_cfg = {
        .mode = TUYA_GPIO_IRQ_FALL,
        .cb = __dev_pin_irq_handle,
        .arg = NULL,
    };
    tal_gpio_irq_init(KEY_GPIO, &irq_cfg);

}


/**
 * @brief
 *
 * @param timer_id
 * @param arg
 * @return VOID_T
 */
VOID_T app_button_scan_cb(TIMER_ID timer_id, VOID_T *arg)
{
    TUYA_GPIO_LEVEL_E v_level = TUYA_GPIO_LEVEL_HIGH;

    if (OPRT_OK != tal_gpio_read(KEY_GPIO, &v_level))
    {
        return;
    }

    if (v_level == TUYA_GPIO_LEVEL_HIGH)
    {
        sg_key_scan_cnt = 0;
    }
    else
    {
        sg_key_scan_cnt++;
        if (sg_key_scan_cnt >= KEY_SCAN_MAX_CNT)
        {
            tal_sw_timer_start(etimer_join_start_delay, 50, TAL_TIMER_ONCE);
            return;
        }
        tal_sw_timer_start(etimer_key_scan, 20, TAL_TIMER_ONCE);
    }
}



/**
 * @brief Get the option value object
 * 
 * @param cluster
 * @param optionMask
 * @param optionOverride
 * @return UCHAR_T
 */
UCHAR_T app_zcl_get_option_value(ZG_CLUSTER_ID_E cluster, UCHAR_T optionMask, UCHAR_T optionOverride)
{
    UCHAR_T option = 0;
    UCHAR_T state = 0;

    state = tal_zg_read_attribute(1, cluster, OPTION_ATTRIBUTE_ID, &option, sizeof(option));
    if (state != OPRT_OK)
    {
        option = 0;
    }

    if (optionMask == 0x01)
    {
        if (optionOverride == 0x01)
        {
            option = 0x01;
        }
        else
        {
            option = 0x00;
        }
    }
    if (option != 0)
    {
        option = 1;
    }
    return option;
}
