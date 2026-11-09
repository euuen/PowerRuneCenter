/*
按键模块测试（按下时驱动 M3508 旋转）
代码测试完毕，没有发现问题。
*/

#include <c_board_bsp/can.h>
#include <c_board_bsp/key.h>
#include <c_board_bsp/sys.h>
#include <c_board_bsp/usart.h>
#include <cmsis_os.h>
#include <euuen_mod/debug/Logger.h>
#include <euuen_mod/motor/DJI_M3508.h>
#include <stdint.h>

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
  Logger_Log(&logger, "key inited\n");
}

void APP_Loop() {
  if (BSP_Key_Is_Pressed()) {
    DJI_M3508_SetCurrent(&dji_m3508, 500);
    Logger_Log(&logger, "pressed\n");
  } else {
    DJI_M3508_Stop(&dji_m3508);
  }
  osDelay(10);
}
