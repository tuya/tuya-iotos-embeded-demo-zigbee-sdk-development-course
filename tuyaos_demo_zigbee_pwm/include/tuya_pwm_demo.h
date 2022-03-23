#ifndef __TUYA_PWM_DEMO_H__
#define __TUYA_PWM_DEMO_H__

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

#define PWM_FREQUENCY_1000 1000
#define PWM_DUTY_500 500
#define TWO_PWM_CHANNELS 2

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

  typedef struct
  {
    UCHAR_T is_warm_start; //
    UCHAR_T rst_cnt;
    UCHAR_T padding[2];
  } rst_info_t;

  typedef struct
  {
    USHORT_T H;
    USHORT_T S;
    USHORT_T V;
  } app_hsv_t;

  typedef struct
  {
    UCHAR_T h;
    UCHAR_T s;
    UCHAR_T v;
  } zigbee_hsv_t;

  typedef struct
  {
    UCHAR_T onoff;
    UINT16_T bright;
    UINT16_T temperature;
  } app_data_t;

  typedef struct
  {
    UCHAR_T onoff;
    UCHAR_T mode;
    UINT16_T H;
    UINT16_T S;
    UINT16_T V;
    UINT16_T B;
    UINT16_T T;
  } app_all_data_t;

  typedef enum
  {
    ZIGBEE_CMD_SINGLE = 0,
    ZIGBEE_CMD_GROUP
  } ZIGBEE_CMD_T;

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

  /*
* @note user pwm init
* @param [in] {user_pwm_init_t} pwm_init
* @return: none
*/
  OPERATE_RET app_pwm_init(VOID_T);

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

  extern UINT8_T g_power_on_state;
  extern UINT16_T g_power_on_bright;
  extern UINT16_T g_power_on_temperature;



  /**
 * @note Light control switch change mode enum
 *          SWITCH_GRADUAL -> turn on/off gradually
 *          SWITCH_DIRECT  -> turn on/off directly
 */
  typedef enum
  {
    SWITCH_GRADUAL = 0,
    SWITCH_DIRECT,
    SWITCH_MAX,
  } CTRL_SWITCH_MODE_E;

  /**
 * @note Light control drive mode enum
 *          BRIGHT_MODE_CW  -> CW drive by pwm totally
 *          BRIGHT_MODE_CCT -> C value is bright setting essentially , w is the scale of C&W .
 *                          the light will send warm, when w is set by zero.
 */
  typedef enum
  {
    BRIGHT_MODE_CW = 0,
    BRIGHT_MODE_CCT,
    BRIGHT_MODE_MAX,
  } CTRL_BRIGHT_MODE_E;

  /**
 * @note Light control data structure
 * storage the light control data(normal issued by app)
 */
  typedef enum
  {
    WHITE_MODE = 0,
    COLOR_MODE,
    SCENE_MODE,
    MUSIC_MODE,
    MODE_MAX,
  } LIGHT_MODE_E;

  /**
 * @note Light control real time control change mode enum
 */
  typedef enum
  {
    REALTIME_CHANGE_JUMP = 0,
    REALTIME_CHANGE_SHADE,
  } REALTIME_CHANGE_E;

  /**
 * @note Light control data structure
 * storage the light control data(normal issued by app)
 */
  typedef struct
  {
    UINT8_T onoff_status; /* on off setting */
    LIGHT_MODE_E mode;
    USHORT_T bright;
    USHORT_T temper;
    ULONG_T count_down_cnt;
  } light_ctrl_data_t;

  typedef enum
  {
    SHADE_PARAM_RGBCW = 0,
    SHADE_PARAM_B,
    SHADE_PARAM_T,
    SHADE_PARAM_XY,
  } SHADE_PARAM_E;

  typedef struct
  {
    USHORT_T bright;
    USHORT_T temper;
  } color_white_t;

  /**
 * @note Bright 5ways value structure
 * Used in light gradually change calculation process
 */
  typedef struct
  {
    USHORT_T red;
    USHORT_T green;
    USHORT_T blue;
    USHORT_T white;
    USHORT_T warm;
  } light_data_t;

  typedef struct
  {
    UINT8_T first_change_flag; ///< shade start, need do somethine first
    SHADE_PARAM_E shade_param; ///< What param of light to be shade
    FLOAT_T slope;             ///<
    light_data_t target_val;   // Target RGBCW output
    light_data_t current_val;  // Current RGBCW output, Used for R, G, B ,C, W shade, when light mode change or rgb change (color mode = rgb)

    color_white_t current_white; // Current level and temper, Used for B, T shade when BT change
    color_white_t target_white;  // Current level and temper, Used for B, T shade when BT change

  } light_ctrl_handle_t;

  /**
 * @note: set light count down cnt
 * @param [in] {ULONG_T}count
 * @return: none
 */
  VOID_T app_light_count_down_cnt_set(ULONG_T count);

  /**
 * @note: return app_light_count_down_cnt_get
 * @param [in] none
 * @return: app_light_count_down_cnt_get
 */
  ULONG_T app_light_count_down_cnt_get(VOID_T);

  /**
 * @note: calculate the CW Light_Handle.TargetVal according to
 *          the Light_Data value, and cw output limit and amplify process!
 * @param [in] {USHORT_T Bright -> Light_Data bright }
 * @param [in] {USHORT_T Temper -> Light_Data temperature}
 * @param [out] {light_data_t *Result -> calc result}
 * @return: none
 */
  VOID_T app_light_ctrl_data_calc_cw(USHORT_T bright, USHORT_T temper, light_data_t *result);

  /**
 * @berief: set light switch data, adapte control data issued by system
 *          to control data format.
 * @param [in] {UINT8_T bONOFF -> switch data, TRUE will turn on}
 * @return: OPERATE_RET -> OPRT_OK meaning need to call app_light_ctrl_proc() function!
 */
  OPERATE_RET app_light_ctrl_data_switch_set(UINT8_T on_off);

  /**
 * @berief: set light mode data
 * @param [in ]{LIGHT_MODE_E Mode}
 * @attention:Mode value is below:
 *                                  WHITE_MODE = 0,
 *                                  COLOR_MODE = 1,
 *                                  SCENE_MODE = 2,
 *                                  MUSIC_MODE = 3,
 * @return: OPERATE_RET -> OPRT_OK meaning need to call app_light_ctrl_proc() function!
 */
  OPERATE_RET app_light_ctrl_data_mode_set(LIGHT_MODE_E mode);

  /**
 * @note: get light switch data
 * @param {OUT UINT8_T *onff -> switch data return}
 * @return: OPERATE_RET
 */
  USHORT_T app_light_ctrl_handle_data_bright_get(VOID_T);

  /**
 * @berief: set light bright data, adapte control data issued by system
 *          to control data format.
 * @param [in] {IN USHORT_T usBright}
 * @attention: acceptable range:10~1000
 * @attention: set bright value, will auto set the Mode to WHITE_MODE !
 * @return: OPERATE_RET -> OPRT_OK meaning need to call app_light_ctrl_proc() function!
 */
  OPERATE_RET app_light_ctrl_data_bright_set(USHORT_T bright, USHORT_T transtime);

  /**
 * @berief: set light temrperature data, adapte control data issued by system
 *          to control data format.
 * @param [in] {IN USHORT_T usTemperature}
 * @attention: acceptable range:0~1000
 * @return: OPERATE_RET -> OPRT_OK meaning need to call app_light_ctrl_proc() function!
 */
  OPERATE_RET app_light_ctrl_data_temp_set(USHORT_T temperature, USHORT_T transtime);

  /**
 * @berief: get light switch data
 * @param [out] {OUT UINT8_T *onpONOFFoff -> switch data return}
 * @return: OPERATE_RET
 */
  OPERATE_RET app_light_ctrl_data_switch_get(UINT8_T *on_off);

  /**
 * @berief: geta light bright data
 * @param [out] {OUT USHORT_T *Bright -> bright data return}
 * @return: OPERATE_RET
 */
  OPERATE_RET app_light_ctrl_data_bright_get(USHORT_T *bright);

  /**
 * @berief: get light temrperature data
 * @param [out] {OUT USHORT_T *Temperature -> temperature data return}
 * @return: OPERATE_RET
 */
  OPERATE_RET app_light_ctrl_data_temper_get(USHORT_T *temperature);

  /**
 * @berief: get light mode data
 * @param [out] {OUT LIGHT_MODE_E *Mode -> mode data return}
 * @return: OPERATE_RET
 */
  OPERATE_RET app_light_ctrl_data_mode_get(LIGHT_MODE_E *mode);

  /**
 * @berief: get control ctrl data
 * @param {none}
 * @return: light_ctrl_data_t
 */
  light_ctrl_data_t app_light_ctrl_data_get(VOID_T);

  /**
 * @berief: Light control proc
 * @param {none}
 * @return: OPERATE_RET
 */
  OPERATE_RET app_light_ctrl_proc(VOID_T);

  /**
 * @note app_onoff_count_down_response
 * @param [in] {ULONG_T} remain_time_sec
 * @return: none
 */
  VOID_T app_onoff_count_down_response(ULONG_T remain_time_sec);

  /**
 * @berief: set light countdown value
 * @param {IN INT_T CountDownSec -> unit:second}
 * @attention: countdown lave time will return with
 *              calling app_onoff_count_down_response function every minutes.
 *              switch status will return with calling
 *              light_ctrl_data_switch_response function when countdown active.
 * @return: OPERATE_RET -> OPRT_OK set countdown OK.
 */
  OPERATE_RET app_onoff_data_count_down_set(ULONG_T count_down_cnt);

  /**
 * @berief: Light hardware reboot judge & proc
 *          process detail:
 *                  1. load reboot cnt data;
 *                  2. reboot cnt data increase;
 *                  3. start software time to clear reboot cnt;
 * @param {none}
 * @return: none
 */
  VOID_T dev_reset_cnt_proc(VOID_T);

  /**
 * @berief: system reset proc
 * @param {none}
 * @attention: this function need implememt by system,
 *              need to deal with different thing in each plantform.
 * @return: none
 */
  OPERATE_RET light_sys_reset_cnt_over_callback(VOID_T);

  /**
 * @description: start blink
 * @param {IN ULONG_T BlinkTimeMs -> blink phase}
 * @attention: blink display will as the parm
 *             -- NetColor, usNetBright in configuration.
 * @return: none
 */
  OPERATE_RET app_light_ctrl_blink_start(ULONG_T blink_time);

  /**
 * @description: stop blink
 * @param {type} none
 * @attention: blink stop will directly go to normal status display
 *              normal status will bright as default bright parm
 *              -- usDefBright,usDefTemper,DefColor in configuration.
 * @return: none
 */
  OPERATE_RET app_light_ctrl_blink_stop(VOID_T);

  /**
 * @description: set PWM channel num
 * @param [in] {UCHAR_T} channel_num
 * @return: none
 */
  VOID_T app_light_ctrl_ch_num_set(UCHAR_T channel_num);

  /**
 * @description: when shade param changed , update light ctrl hand shade param
 * @param [in] {SHADE_PARAM_E} shade_param
 * @return: none
 */
  VOID_T app_light_ctrl_shade_param_set(SHADE_PARAM_E shade_param);

  /**
 * @description: Move to off when shade stop
 * @param {type} bMoveToOff
 * @return:
 */
  VOID_T app_light_ctrl_move_down_to_off_flag_set(VOID_T);

  /**
 * @description: Get shade remain time
 * @param [OUT]{RemainTime100ms} remain time: 100ms
 * @return: none
 */
  VOID_T app_light_ctrl_remain_time_ms_get(USHORT_T *remain_time_100ms);

  /**
 * @description: stop step level or move level
 * @param {type} none
 * @return: none
 */
  OPERATE_RET app_light_ctrl_data_move_stop(VOID_T);

  /**
 * @description: set level_move_down_to_off value
 * @param [in] {UINT8_T} flag
 * @return: none
 */

  VOID_T app_light_ctrl_level_move_down_to_off_set(UINT8_T flag);

  /**
 * @note: set light temrperature data
 * @param [in] {USHORT_T } temperature
 * @return: OPERATE_RET
 */
  VOID_T app_light_ctrl_data_temper_set(USHORT_T temperature);

  VOID_T app_light_ctrl_save_data_delay_cb(TIMER_ID timer_id, VOID_T *arg);
  VOID_T app_light_ctrl_mf_blink_cb(TIMER_ID timer_id, VOID_T *arg);
  VOID_T app_light_ctrl_power_on_sync_cb(TIMER_ID timer_id, VOID_T *arg);
  VOID_T app_light_ctrl_join_end_sync_cb(TIMER_ID timer_id, VOID_T *arg);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __LIHGT_CONTROL_H__ */
