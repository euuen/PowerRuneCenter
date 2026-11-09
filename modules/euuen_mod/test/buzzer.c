/*
蜂鸣器模块测试（按键控制提示音）
代码测试完毕，没有发现问题。
*/

#include <c_board_bsp/buzzer.h>
#include <c_board_bsp/key.h>
#include <c_board_bsp/sys.h>
#include <c_board_bsp/usart.h>
#include <cmsis_os.h>
#include <euuen_mod/debug/Logger.h>
#include <euuen_mod/remote_control/FSi6x.h>
#include <stdio.h>

BSP_USART bsp_usart_1;
Logger logger;

void APP_Setup() {
  BSP_Buzzer_Init();
  BSP_USART_1_Init(&bsp_usart_1);
  Logger_Init(&logger, &bsp_usart_1);
  Logger_Log(&logger, "buzzer inited\n");
}

void APP_Loop() {
  if (BSP_Key_Is_Pressed()) {
    BSP_Buzzer_On();
  } else {
    BSP_Buzzer_Off();
  }
  osDelay(10);
}
