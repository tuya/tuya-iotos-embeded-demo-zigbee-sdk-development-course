/*
 * @FileName: your project
 * @Author: Tuya
 * @Email:
 * @LastEditors: Tuya
 * @Date: 2022-03-07 14:24:24
 * @LastEditTime: 2022-03-22 17:24:50
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

/***********************************************************
************************micro define************************
***********************************************************/
#define adcFreq   10000000    // ADC frequency 10M
#define adc_ch_num 1
//#define adc_width 12

#define ADC_SAMPLE_TIME_MS 1000
#define adc_type TUYA_ADC_INNER_SAMPLE_VOL

BOOL_T g_adc_stype = FALSE;
BOOL_T g_user_adc_init_flag = FALSE;     //STATIC

STATIC ULONG_T sg_sample_time_ms = 0;
STATIC UCHAR_T sg_adc_channel = 0;

STATIC TUYA_ADC_BASE_CFG_T sg_adc_init_cfg;    //
//STATIC BOOL_T g_user_adc_init_flag = FALSE;    //STATIC

TIMER_ID etimer_adc_sample;

UINT8_T ADC_LIST[] = {10};

//extern OPERATE_RET tkl_adc_mapping_to_gpio(UINT8_T ch_id, UINT8_T unit_num);


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



VOID_T adc_ch_num_set(UCHAR_T unit_num)
{
    sg_adc_channel = unit_num;
}



OPERATE_RET tuya_adc_init(VOID_T)
{
    OPERATE_RET v_ret = OPRT_COM_ERROR;

    UINT32_T *adc_data;
    UINT32_T volt = 0;
    FLOAT_T temp = 0.0;
    UINT8_T adc_width = 0;

    TUYA_ADC_BASE_CFG_T adc_cfg = {
    .ch_list = ADC_LIST,
    .ch_nums = adc_ch_num,
    .width = 0,
    .type = adc_type,
};

    //tkl_adc_mapping_to_gpio(1, 10);

    v_ret = tkl_adc_init(ADC_LIST[0], &adc_cfg);

    v_ret = tkl_adc_temperature_get();

    //v_ret = tkl_adc_read_data(ADC_LIST[0], adc_data, sizeof(adc_data));

    if (v_ret != OPRT_OK) {
       TAL_PR_DEBUG("adc init error!");
    }
 
    volt = tkl_adc_ref_voltage_get();
    adc_width = tkl_adc_width_get(ADC_LIST[0]);
    //temp = tkl_adc_temperature_get();


    g_user_adc_init_flag = TRUE;
    g_adc_stype = adc_type;
    adc_ch_num_set(adc_ch_num);

    TAL_PR_DEBUG("adc init ok!\n");

    TAL_PR_DEBUG("volt: %d\n", volt);
    TAL_PR_DEBUG("adc_width: %d\n", adc_width);
    //TAL_PR_DEBUG("volt: %d\n, temp: %f\n", volt, temp);

    return v_ret;
}


/**
 * @note: light ctrl blink timer callback
 * @param {none}
 * @return: none
 */
VOID_T adc_timer_cb(TIMER_ID timer_id, VOID_T *arg)
{
    //tuya_get_adc_vol();
    adc_ctrl_sample_start(1000);
    tal_sw_timer_start(etimer_adc_sample, sg_sample_time_ms, TAL_TIMER_ONCE);

}

OPERATE_RET adc_ctrl_sample_start(ULONG_T sample_time)
{
    sg_sample_time_ms = 1000;
    tal_sw_timer_start(etimer_adc_sample, 10, TAL_TIMER_ONCE);
    TAL_PR_DEBUG("");

    return OPRT_OK;

    OPERATE_RET ret =-1;
    STATIC UCHAR_T cnt = 0;
    adc_data_t adc_data;

    tal_system_memset(&adc_data, 0, sizeof(adc_data));
    adc_data.vol = 0;
    ret = tkl_adc_ref_voltage_get();
    TAL_PR_DEBUG("adc_data.vol: %d\n", adc_data.vol);
    // if (ret != OPRT_OK)
    // {
    //     TAL_PR_DEBUG("ADC VOL error");
    // }
    // cnt++;
}
