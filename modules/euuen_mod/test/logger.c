/*
Logger 模块测试（字符串 + JustFloat 协议）
代码测试完毕，没有发现问题。
*/

#include <c_board_bsp/sys.h>
#include <c_board_bsp/usart.h>
#include <cmsis_os.h>
#include <euuen_mod/debug/Logger.h>
#include <euuen_mod/motor/DJI_M3508.h>
#include <stdint.h>
#include <stdio.h>

BSP_CAN bsp_can_1;
BSP_USART bsp_usart_1;
Logger logger;
DJI_M3508 dji_m3508;

void APP_Setup() {
  BSP_CAN_1_Init(&bsp_can_1);
  BSP_CAN_1_AddRxCallback(DJI_M3508_RxCallback);
  BSP_USART_1_Init(&bsp_usart_1);
  Logger_Init(&logger, &bsp_usart_1);
  DJI_M3508_Init(&dji_m3508, &bsp_can_1, 1);
  Logger_Log(&logger, "logger inited\n");
}

void APP_Loop() {
  // 打印电机反馈，用于验证 Logger 输出
  char out_str[64];
  snprintf(out_str, sizeof(out_str), "ecd=%u speed=%.2f cur=%d\n",
           dji_m3508.measure.ecd, dji_m3508.measure.speed_aps,
           dji_m3508.measure.real_current);
  Logger_Log(&logger, out_str);
  osDelay(50);
}
