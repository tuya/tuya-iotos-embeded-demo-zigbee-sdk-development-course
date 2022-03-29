/*
 * @FileName: your project
 * @Author: Tuya
 * @Email:
 * @LastEditors: Tuya
 * @Date: 2022-03-07 14:24:24
 * @LastEditTime: 2022-03-29 11:48:36
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company:  http://www.tuya.com
 * @Description:
 */

#include "tal_system.h"
#include "tal_gpio.h"
#include "tuya_adc_demo.h"
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
#define adcFreq   10000000    // ADC frequency 10M
#define adc_ch_num 2
#define adc_width 12

#define ADC_SAMPLE_TIME_MS 1000
#define adc_type TUYA_ADC_INNER_SAMPLE_VOL

//STATIC ULONG_T sg_sample_time_ms = 0;
STATIC UCHAR_T sg_adc_channel = 0;
STATIC TUYA_ADC_BASE_CFG_T sg_adc_init_cfg;
STATIC BOOL_T g_user_adc_init_flag = FALSE;    //STATIC


TIMER_ID etimer_adc_sample;

UINT8_T ADC_LIST[] = {10,17};
BOOL_T g_adc_stype = FALSE;

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


OPERATE_RET tuya_adc_init(VOID_T)
{
    TUYA_ADC_BASE_CFG_T adc_cfg = {
    .ch_list = ADC_LIST,
    .ch_nums = adc_ch_num,
    .width = adc_width,
    .type = adc_type,
};

    tkl_adc_init(10, &adc_cfg);
    tkl_adc_init(17, &adc_cfg);

    TAL_PR_DEBUG("adc init ok!\n");
    return OPRT_OK;
}

/**
 * @note: adc sample timer callback
 * @param {none}
 * @return: none
 */

VOID_T adc_sample_cb(TIMER_ID timer_id, VOID_T *arg)
{
    OPERATE_RET ret;

    UINT32_T *adc_data;
    UINT32_T *adc_con_buff;
    UINT32_T volt = 0;
    INT32_T temp = 0;
    UINT8_T sample_width = 0;

    //ret = tkl_adc_temperature_get();

    ret = tkl_adc_read_data(10, adc_data, SIZEOF(adc_data));

    tal_system_memset(&adc_con_buff, 0, SIZEOF(adc_data));

    if (ret != OPRT_OK) {
       TAL_PR_DEBUG("adc sample failed: %d", ret);
       //TAL_PR_DEBUG("adc1 sample failed: %d", ret1);
       TAL_PR_DEBUG("adc_data :%d\n", adc_data);

    }
    tkl_adc_read_single_channel(10, ADC_LIST[0], adc_con_buff);
    TAL_PR_DEBUG("adc_con_buff: %d\n", adc_con_buff);

    volt = tkl_adc_ref_voltage_get();
    sample_width = tkl_adc_width_get(10);
    //temp = tkl_adc_temperature_get();

    g_user_adc_init_flag = TRUE;
    g_adc_stype = adc_type;

    TAL_PR_DEBUG("volt: %d\n mv", volt);
    TAL_PR_DEBUG("sample_width: %d\n", sample_width);
    //TAL_PR_DEBUG("temp : %d\n", temp);
    //return ret;

    tal_sw_timer_start(etimer_adc_sample, 5000, TAL_TIMER_ONCE);
}
