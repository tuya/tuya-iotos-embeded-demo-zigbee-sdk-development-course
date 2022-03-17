#ifndef __APP_COMMON_H__
#define __APP_COMMON_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include <string.h>
#include "tkl_uart.h"
#include "tal_sw_timer.h"
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

#define KEY_GPIO GPIO_PC4
#define LED_GPIO GPIO_PC5


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

    typedef struct
  {
    UCHAR_T is_warm_start; //
    UCHAR_T rst_cnt;
    UCHAR_T padding[2];
  } rst_info_t;


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


  typedef struct
  {
    UCHAR_T onoff;
    UINT16_T bright;
  } app_data_t;

  typedef struct
  {
    UCHAR_T onoff;
    UCHAR_T mode;
  } app_all_data_t;


  typedef enum
  {
    ZIGBEE_CMD_SINGLE = 0,
    ZIGBEE_CMD_GROUP
  } ZIGBEE_CMD_T;

/**
 * enumerate led mode
 */
typedef enum{
    LED_MODE_NONE = 0,
    LED_MODE_RELAY,
    LED_MODE_POS
}LED_MODE_T;



/**
 * @enum Represents the output state of GPIO, OFF represents invalid, and ON represents valid
 */
typedef enum {
    DEV_IO_OFF = 0, ///< The output level is not equal to gpio_config_t->drive_flag
    DEV_IO_ON,      ///< The output level is equal to gpio_config_t->drive_flag
} DEV_IO_ST_T;


//STATIC UCHAR_T sg_light_channel = 0;
  /**************************************************************************
 *                              init functions
 * ************************************************************************/
  /**
 * @berief: light init
 * @param [in] none 
 * @return: OPERATE_RET
 */
  OPERATE_RET app_light_init(VOID_T);

  /**************************************************************************
 *                          software timer functions
 * ************************************************************************/
  /**
 * @brief software timer use id enum
 */
  typedef enum
  {
    CLEAR_RESET_CNT_SW_TIMER = 35, ///< basic timer id 35
    BLINK_SW_TIMER,
    COUNTDOWN_SW_TIMER,
    SHADE_TIMER,
    NETWORK_JOIN_START_DELAY_TIMER,
  } SW_TIMER_ID_E;



  /**************************************************************************
 *                        pwm functions
 * ************************************************************************/

  /*
* @note user button init
* @param [in] none
* @return: none
*/
  VOID_T app_button_init(VOID_T);

  VOID_T app_button_scan_cb(TIMER_ID timer_id, VOID_T *arg);

  /**
* @brief 
* 
* @param str 
* @return VOID_T 
*/
  VOID_T dev_uart_output(IN CONST CHAR_T *str);

  /*
* @note user gpio init
* @param [in] {user_gpio_init_t} gpio_init
* @return: none
*/
  OPERATE_RET app_gpio_init(VOID_T);

  OPERATE_RET app_light_ctrl_proc(VOID_T);

  OPERATE_RET app_light_ctrl_data_switch_set(UINT8_T on_off);

//extern OPERATE_RET app_light_ctrl_blink_start(ULONG_T blink_time);
// /**
//  * @brief Set the pwm channel num object
//  * 
//  * @param channel_num 
//  */
//   VOID_T app_light_ctrl_ch_num_set(UCHAR_T channel_num)
// {
//     sg_light_channel = channel_num;
// }


STATIC VOID_T netled_status_task(VOID_T);


/**************************************************************************
 *                          flash functions
 * ************************************************************************/
#define RESET_CNT_OFFSET 0
#define LIGHT_APP_DATA_OFFSET (RESET_CNT_OFFSET + 2)
#define PROD_TEST_DATA_OFFSET (LIGHT_APP_DATA_OFFSET + sizeof(light_app_data_flash_t) + 1)

  extern UINT8_T g_power_on_state;
  extern UINT16_T g_power_on_bright;


  /**
 * @description: 获取最终的option值,单个控制指令在关灯状态下是否生效,需要结合option属性值,以及
 * 当前指令的optionMask+optionOverride参数.当前只处理optionMask=0000 0001 的情况,以后联盟标准
 * 拓展了,再更新本函数。
 * @param {cluter} cluster id
 * @param {optionMask} 
 * @param {optionOverride} 
 * @return: option
 */
  UCHAR_T app_zcl_get_option_value(ZG_CLUSTER_ID_E cluster, UCHAR_T optionMask, UCHAR_T optionOverride);

#ifdef __cplusplus
  extern "C"
  {
#endif
#endif
