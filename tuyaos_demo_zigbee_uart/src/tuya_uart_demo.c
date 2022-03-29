/*
 * @FileName: your project
 * @Author: Tuya
 * @Email:
 * @LastEditors: Tuya
 * @Date: 2022-03-07 14:24:24
 * @LastEditTime: 2022-03-29 11:36:46
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company:  http://www.tuya.com
 * @Description:
 */
#include "tal_uart.h"
#include "tal_log.h"
#include "tal_system.h"
#include "tuya_uart_demo.h"

// Size of the buffer for received data
#define BUF_SIZE 256

OPERATE_RET user_uart_init(VOID_T)
{
    TAL_UART_CFG_T uart_cfg = {
        .rx_buffer_size = 256,
        .open_mode = 0,
        {
            .baudrate = 115200,
            .parity = TUYA_UART_PARITY_TYPE_NONE,
            .databits = TUYA_UART_DATA_LEN_8BIT,
            .stopbits = TUYA_UART_STOP_LEN_1BIT,
            .flowctrl = TUYA_UART_FLOWCTRL_NONE,
        }
    };
    tal_uart_init(USER_UART0, &uart_cfg);
}

VOID_T uart0_rx_cb(UINT32_T port_id, VOID_T *buff, UINT16_T len)
{
    if (len > 0 ) {

        tal_uart_write(USER_UART0, buff,  len);

        TAL_PR_DEBUG("I received  %s", buff);

    } else {
        TAL_PR_DEBUG("recv data failed!");

    }
}
OPERATE_RET user_uart_task(VOID_T)
{
    OPERATE_RET ret;
    UINT8_T uart0_rx_buf[BUF_SIZE];

    ret = tal_uart_read(USER_UART0, uart0_rx_buf, SIZEOF(uart0_rx_buf));

        if (ret > 0) {
            tal_uart_write(USER_UART0, uart0_rx_buf,  ret);
        } else {
            tal_uart_write(USER_UART0, "no recv data\r\n", 15);
        }

    tal_uart_rx_reg_irq_cb(USER_UART0, uart0_rx_cb);

}
