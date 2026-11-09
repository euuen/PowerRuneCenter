/*
代码测试完毕，没有发现问题。
*/

#include <c_board_bsp/can.h>
#include <c_board_bsp/sys.h>
#include <c_board_bsp/usart.h>
#include <cmsis_os.h>
#include <euuen_mod/debug/Logger.h>
#include <euuen_mod/remote_control/FSi6x.h>
#include <stdio.h>

BSP_CAN bsp_can_1;
BSP_USART bsp_usart_1;
Logger logger;
FSi6x_Data fsi6x_data;

void board_rx_callback(uint32_t can_id, uint8_t *data, uint8_t length) {
  if (can_id == 0x201) {
    char out_str[64];
    // sprintf(out_str, "%d %d %d %d %d %d %d %d", data[0], data[1], data[2],
    // data[3], data[4], data[5], data[6] ,data[7]);

    uint16_t key_bits = ((uint16_t)data[1] << 8) | data[0];
    int len = 0;

    // 打印9个按键状态（bit0~bit8）
    for (int i = 0; i < 9; i++) {
      int state = (key_bits >> i) & 0x01;
      len += snprintf(out_str + len, sizeof(out_str) - len, "%d ", state);
    }

    // 追加第三个字节的数据（以十进制打印）
    len += snprintf(out_str + len, sizeof(out_str) - len, "%d", data[2]);

    Logger_Log(&logger, out_str);
  }
}

void APP_Setup() {
  BSP_CAN_1_Init(&bsp_can_1);
  BSP_USART_1_Init(&bsp_usart_1);
  Logger_Init(&logger, &bsp_usart_1);
  BSP_CAN_1_AddRxCallback(board_rx_callback);
  Logger_Log(&logger, "center inited");
}

void APP_Loop() { osDelay(100); }
