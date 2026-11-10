/*
FSi6x 遥控器模块测试
代码测试完毕，没有发现问题。
*/

#include <c_board_bsp/sys.h>
#include <c_board_bsp/usart.h>
#include <cmsis_os.h>
#include <euuen_mod/debug/Logger.h>
#include <euuen_mod/remote_control/FSi6x.h>
#include <stdint.h>
#include <stdio.h>

BSP_USART bsp_usart_1;
Logger logger;
FSi6x_Data fsi6x_data;

void APP_Setup() {
  BSP_USART_1_Init(&bsp_usart_1);
  Logger_Init(&logger, &bsp_usart_1);

  FSi6x_Init(&fsi6x_data);
  Logger_Log(&logger, "fsi6x inited\n");
}

void APP_Loop() {
  FSi6x_Read(&fsi6x_data);

  char out_str[128];
  snprintf(out_str, sizeof(out_str), "rx=%.2f ry=%.2f swa=%d swd=%d vra=%.2f is_disconnectd=%d\n",
           fsi6x_data.right_stick_x, fsi6x_data.right_stick_y,
           fsi6x_data.swa, fsi6x_data.swd, fsi6x_data.vra, FSi6x_IsDisconnectd());
  Logger_Log(&logger, out_str);
  osDelay(20);
}
