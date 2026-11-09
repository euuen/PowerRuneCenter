/*
BSP CAN 测试（回环，发送并打印接收）
代码测试完毕，没有发现问题。
*/

#include <c_board_bsp/can.h>
#include <c_board_bsp/sys.h>
#include <c_board_bsp/usart.h>
#include <cmsis_os.h>
#include <stdint.h>
#include <stdio.h>

BSP_CAN bsp_can_1;
BSP_USART bsp_usart_1;

void board_rx_callback(uint32_t can_id, uint8_t *data, uint8_t length) {
  char out_str[32];
  snprintf(out_str, sizeof(out_str), "rx:0x%x len=%u\n", (unsigned)can_id, length);
  BSP_USART_SendStr_BLOCKED(&bsp_usart_1, out_str, 10);
}

void APP_Setup() {
  BSP_CAN_1_Init(&bsp_can_1);
  BSP_CAN_1_AddRxCallback(board_rx_callback);
  BSP_USART_1_Init(&bsp_usart_1);
}

void APP_Loop() {
  uint8_t data[8] = {0xde, 0xad, 0xbe, 0xef, 0, 0, 0, 0};
  BSP_CAN_SendStdFrame(&bsp_can_1, 0x101, data, 8, 100);
  osDelay(100);
}