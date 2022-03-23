/*
 * @FileName: your project
 * @Author: Tuya
 * @Email: 
 * @LastEditors: Tuya
 * @Date: 2022-03-09 10:51:49
 * @LastEditTime: 2022-03-22 17:18:01
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company:  http://www.tuya.com
 * @Description: 
 */


#ifndef __TUYA_ADC_DEMO_H__
#define __TUYA_ADC_DEMO_H__


#ifdef __cplusplus
    extern "C" {
#endif

#include <string.h>
#include "tkl_uart.h"
#include "tal_sw_timer.h"
#include "tal_system.h"
#include "tal_adc.h"
#include "tkl_adc.h"
#include "tuya_cloud_types.h"
#include "tuya_zigbee_stack.h"

/**************************************************************************
 *                              global  variable
 * ************************************************************************/
#define GPIO_PA0        0
#define GPIO_PA1        1
#define GPIO_PA2        2
#define GPIO_PA3        3
#define GPIO_PA4        4
#define GPIO_PA5        5
#define GPIO_PA6        6
#define GPIO_PB0        7
#define GPIO_PB1        8
#define GPIO_PC0        9
#define GPIO_PC1        10
#define GPIO_PC2        11
#define GPIO_PC3        12
#define GPIO_PC4        13
#define GPIO_PC5        14
#define GPIO_PD0        15
#define GPIO_PD1        16
#define GPIO_PD2        17
#define GPIO_PD3        18
#define GPIO_PD4        19

#define USER_UART0      0

#define MG21_FLASH_PAGE_SIZE (8 * 1024UL)
#define FLASH_ADDR_RST_CNT (0xE4000UL)
#define FLASH_ADDR_APP_DATA (FLASH_ADDR_RST_CNT + MG21_FLASH_PAGE_SIZE)

#define ZCL_ID_ONOFF 100
#define ZCL_ID_MODE 101
#define ZCL_ID_BRIGHT 102
#define ZCL_ID_TEMPER 103
#define ZCL_ID_COUNT_DOWN 104
#define ZCL_ID_APP_DATA 105


#define C_NUM 5  // b5
#define W_NUM 10 // c2   pin num refer to soc_gpio.c

#define ZIGBEE_JOIN_MAX_TIMEOUT 30 * 1000
#define ZIGBEE_HEAR_BEAT_DURATION 150 * 1000 ///< 150s ~180s
#define NETWORK_JOIN_START_DELAY_TIME 1000   ///< ms
#define APP_CMD_HANDLE_DELAY_TIME 500

#define OPTION_ATTRIBUTE_ID 0x000F

#define ZIGBEE_JOIN_MAX_TIMEOUT_MS 180 * 1000UL
#define BEACON_SEND_INTERVAL_200MS 200

#define LED_MODE_ATTRIBUTE_ID         0x8001

#define 		LED_1_IO_INDEX              	0

#define DEV_LED_BLINK_FOREVER 0xFFFF


///< level cluster
#define TY_ATTR_CURRENT_BRIGHT_ATTR_ID 0xF000
///< on off cluster
#define TY_ATTR_COUNT_DOWN_ATTR_ID 0xF000

#if defined(EMBER_AF_PRINT_ENABLE) && defined(TUYA_RUNTIME_DBG)
#include "tkl_uart.h"
#define tal_main_debug(...) tkl_dbg_printf(__VA_ARGS__)
#else
#define tal_main_debug(...) tkl_dbg_printf(__VA_ARGS__)
#endif

#ifndef OPRT_OK
#define OPRT_OK 0
#endif
#ifndef OPRT_INVALID_PARM
#define OPRT_INVALID_PARM 1
#endif
#ifndef OPRT_COM_ERROR
#define OPRT_COM_ERROR 2
#endif

#define PIN_NOEXIST 0xFF ///< default pin def

  typedef enum{
    REPORT_ZCL_ID_ONOFF = 100,
    REPORT_ZCL_ID_POWER_STATUS,
    REPORT_ZCL_ID_LED_MODE,
    REPORT_ZCL_ID_ONOFF_TIME
  } ZCL_ID_T;

  typedef enum{
    DEV_WORK_ST_NORMAL = 0,
    DEV_WORK_ST_
  } DEV_WORK_ST_T;


  typedef enum
  {
    ZIGBEE_CMD_SINGLE = 0,
    ZIGBEE_CMD_GROUP
  } ZIGBEE_CMD_T;

  typedef struct
  {
    UINT32_T vol;
    FLOAT_T temp;
    UINT8_T width;
  } adc_data_t;


  /**************************************************************************
 *                              init functions
 * ************************************************************************/
  /**



  /**
* @brief
*
* @param str
* @return VOID_T
*/
  VOID_T dev_uart_output(IN CONST CHAR_T *str);


OPERATE_RET tuya_adc_init(VOID_T);

VOID_T adc_timer_cb(TIMER_ID timer_id, VOID_T *arg);
VOID_T adc_ch_num_set(UCHAR_T channel_num);

OPERATE_RET adc_ctrl_sample_start(ULONG_T sample_time);

/**************************************************************************
 *                          flash functions
 * ************************************************************************/
#define RESET_CNT_OFFSET 0
#define LIGHT_APP_DATA_OFFSET (RESET_CNT_OFFSET + 2)
#define PROD_TEST_DATA_OFFSET (LIGHT_APP_DATA_OFFSET + sizeof(light_app_data_flash_t) + 1)



#ifdef __cplusplus
}
#endif

#endif
