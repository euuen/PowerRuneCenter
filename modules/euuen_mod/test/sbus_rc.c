/*
SBUS 接收机模块测试
代码测试完毕，没有发现问题。
*/

#include <c_board_bsp/sys.h>
#include <c_board_bsp/usart.h>
#include <cmsis_os.h>
#include <euuen_mod/debug/Logger.h>
#include <euuen_mod/remote_control/SBUS_RC.h>
#include <stdint.h>
#include <stdio.h>

BSP_USART bsp_usart_1;
Logger logger;

void APP_Setup() {
  BSP_USART_1_Init(&bsp_usart_1);
  Logger_Init(&logger, &bsp_usart_1);

  SBUS_RC_Init();
  Logger_Log(&logger, "sbus rc inited\n");
}

void APP_Loop() {
  // 直接读取 SBUS 原始通道数据
  char out_str[64];
  int len = 0;
  for (int i = 0; i < 4; i++) {
    len += snprintf(out_str + len, sizeof(out_str) - len, "%d ", sbus_rc.channels[i]);
  }
  Logger_Log(&logger, out_str);
  osDelay(20);
}
