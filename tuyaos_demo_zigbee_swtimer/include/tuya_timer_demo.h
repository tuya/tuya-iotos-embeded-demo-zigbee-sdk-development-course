#ifndef __TUYA_TIMER_DEMO_H__
#define __TUYA_TIMER_DEMO_H__

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include <string.h>
#include "tkl_uart.h"
#include "tal_sw_timer.h"
#include "tuya_cloud_types.h"
#include "tuya_zigbee_stack.h"


#define shade_log_en 0

#define HW_TIMER_CYCLE_MS HW_TIMER_CYCLE_US / 1000                   ///< hardware timer cycle (unit:ms)
#define CTRL_HSV_H_MAX 360                                           ///< Control calculate range 0 ~ 360
#define CTRL_CAL_VALUE_RANGE 1000                                    ///< Control calculate range 0 ~ 1000
#define CTRL_CW_BRIGHT_VALUE_MAX CTRL_CAL_VALUE_RANGE                ///< Control cw calc max bright value
#define CTRL_CW_BRIGHT_VALUE_MIN (CTRL_CW_BRIGHT_VALUE_MAX * 0.01)   ///< Control cw calc min bright value (max value * 0.01)
#define CTRL_RGB_BRIGHT_VALUE_MAX CTRL_CAL_VALUE_RANGE               ///< Control RGB calc max bright value
#define CTRL_RGB_BRIGHT_VALUE_MAX CTRL_CAL_VALUE_RANGE               ///< Control RGB calc max bright value
#define CTRL_RGB_BRIGHT_VALUE_MIN (CTRL_RGB_BRIGHT_VALUE_MAX * 0.01) ///< Control RGB calc min bright value
#define HW_TIMER_CYCLE_US 5000                                       ///< hardware timer cycle (unit:us)
#define LIGHT_SHADE_CYCLE 5                                          ///< light shade change cycle (unit:ms)
#define SHADE_CHANG_MAX_TIME 1000                                    ///< shade change max time(the worst situation),uint:ms


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

#define APP_WHITE_B_MAX 1000
#define APP_WHITE_T_MAX 1000
#define APP_COLOR_H_MAX 360
#define APP_COLOR_S_MAX 1000
#define APP_COLOR_V_MAX 1000

#define ZIGBEE_WHITE_B_MAX 255 ///< color: level
#define ZIGBEE_WHITE_T_MAX 255 ///< color: temperature
#define ZIGBEE_COLOR_H_MAX 254 ///< color: Hue
#define ZIGBEE_COLOR_S_MAX 254 ///< color: Saturation
#define ZIGBEE_COLOR_V_MAX 255 ///< color: value

#define ZIGBEE_JOIN_MAX_TIMEOUT 30 * 1000
#define ZIGBEE_HEAR_BEAT_DURATION 150 * 1000 ///< 150s ~180s
#define NETWORK_JOIN_START_DELAY_TIME 1000   ///< ms
#define APP_CMD_HANDLE_DELAY_TIME 500

#define OPTION_ATTRIBUTE_ID 0x000F

#define ZIGBEE_JOIN_MAX_TIMEOUT_MS 180 * 1000UL
#define BEACON_SEND_INTERVAL_200MS 200

#define TY_CMD_SET_BRIGHT_CMD_ID 0xF0
#define TY_CMD_SET_COLOR_MODE_CMD_ID 0xF0          ///< change mode cmd: white/color/scene/music
#define TY_CMD_SET_COLOR_SCENE_CMD_ID 0xF1         ///< scene data cmd
#define TY_CMD_SET_COLOR_REALTIME_DATA_CMD_ID 0xF2 ///< real time ctrl cmd: mode(1) + H(2) + S(2) + V(2) + B(2) + T(2) = 11 bytes
#define TY_CMD_SET_COLOR_TEMPERATURE_CMD_ID 0xE0   ///<  cmd: temperature

///< color cluster
#define TY_ATTR_LIGHT_MODE_ATTRIBUTE_ID 0xF000       ///< light mode attribute (1 byte)
#define TY_ATTR_LIGHT_V_VALUE_ATTRIBUTE_ID 0xF001    ///< color V attribute (1 byte)
#define TY_ATTR_LIGHT_APP_DATA_ATTRIBUTE_ID 0xF102   ///< all app data attribute: mode(1) + H(1) + S(1) + V(1) + B(1) + T(2) = 7 Bytes
#define TY_ATTR_LIGHT_SCENE_DATA_ATTRIBUTE_ID 0xF003 ///< scene data attribute (2+8*n, n<=8) Bytes
#define TY_ATTR_CURRENT_BRIGHT_ATTR_ID 0xF000        ///<
#define TY_ATTR_COLOR_TEMPERATURE_ATTR_ID 0xE000     ///<

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

  typedef enum
  {
    ZIGBEE_CMD_SINGLE = 0,
    ZIGBEE_CMD_GROUP
  } ZIGBEE_CMD_T;

  /**************************************************************************
 *                              init functions
 * ************************************************************************/


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
 *                        gpio functions
 * ************************************************************************/

  /*
* @note user gpio init
* @param [in] {user_gpio_init_t} gpio_init
* @return: none
*/

OPERATE_RET tuya_gpio_init(VOID_T);
  /**
 * @brief
 *
 * @param str
 * @return VOID_T
 */
  VOID_T dev_uart_output(IN CONST CHAR_T *str);

/**************************************************************************
 *                          flash functions
 * ************************************************************************/
#define RESET_CNT_OFFSET 0
#define LIGHT_APP_DATA_OFFSET (RESET_CNT_OFFSET + 2)
#define PROD_TEST_DATA_OFFSET (LIGHT_APP_DATA_OFFSET + sizeof(light_app_data_flash_t) + 1)



  typedef struct
  {
    UINT8_T led_status; /* led setting */
    ULONG_T count_down_cnt;
  } led_data_t;


  /**
 * @berief: get light switch data
 * @param [out] {OUT UINT8_T *onpONOFFoff -> switch data return}
 * @return: OPERATE_RET
 */
  OPERATE_RET app_light_ctrl_data_switch_get(UINT8_T *on_off);

  /**
 * @description: start blink
 * @param {IN ULONG_T BlinkTimeMs -> blink phase}
 * @attention: blink display will as the parm
 *             -- NetColor, usNetBright in configuration.
 * @return: none
 */
  OPERATE_RET led_ctrl_blink_start(ULONG_T blink_time);

  /**
 * @description: stop blink
 * @param {type} none
 * @attention: blink stop will directly go to normal status display
 *              normal status will bright as default bright parm
 *              -- usDefBright,usDefTemper,DefColor in configuration.
 * @return: none
 */
  OPERATE_RET app_light_ctrl_blink_stop(VOID_T);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __LIHGT_CONTROL_H__ */
