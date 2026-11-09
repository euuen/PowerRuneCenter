/*
BSP 蜂鸣器测试
代码测试完毕，没有发现问题。
*/

#include <c_board_bsp/buzzer.h>
#include <c_board_bsp/key.h>
#include <c_board_bsp/sys.h>
#include <cmsis_os.h>

void APP_Setup() {
  BSP_Buzzer_Init();
}

void APP_Loop() {
  if (BSP_Key_Is_Pressed()) {
    BSP_Buzzer_On();
  } else {
    BSP_Buzzer_Off();
  }
  osDelay(10);
}