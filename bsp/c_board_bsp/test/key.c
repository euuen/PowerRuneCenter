/*
BSP 按键测试
代码测试完毕，没有发现问题。
*/

#include <c_board_bsp/key.h>
#include <c_board_bsp/led.h>
#include <c_board_bsp/sys.h>
#include <cmsis_os.h>

void APP_Setup() {
  BSP_LED_Init();
}

void APP_Loop() {
  if (BSP_Key_Is_Pressed()) {
    BSP_LED_Set_RGB(255, 0, 0);
  } else {
    BSP_LED_Set_RGB(0, 0, 0);
  }
  osDelay(10);
}