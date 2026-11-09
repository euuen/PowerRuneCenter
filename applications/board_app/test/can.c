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
    char out_str[32];
    sprintf(out_str, "%d %d %d %d", data[0], data[1], data[2], data[3]);
    Logger_Log(&logger, out_str);
  }
}

void APP_Setup() {
  BSP_CAN_1_Init(&bsp_can_1);
  BSP_USART_1_Init(&bsp_usart_1);
  Logger_Init(&logger, &bsp_usart_1);
  BSP_CAN_1_AddRxCallback(board_rx_callback);
  Logger_Log(&logger, "center inited");
  FSi6x_Init(&fsi6x_data);
}

void APP_Loop() {
  FSi6x_Read(&fsi6x_data);
  uint8_t data[8] = {0};
  data[0] = fsi6x_data.swa;
  data[1] = fsi6x_data.swb;
  float mapped_vra = fsi6x_data.vra * 10.0f; // 1.0 * 10 = 10.0
  data[2] = (uint8_t)(mapped_vra + 0.5f);    // 四舍五入，结果为 10
  BSP_CAN_SendStdFrame(&bsp_can_1, 0x101, data, 8, 100);
  osDelay(100);
}
