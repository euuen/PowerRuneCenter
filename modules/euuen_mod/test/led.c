/*
LED 灯条模块测试（RGB 渐变）
代码测试完毕，没有发现问题。
*/

#include <c_board_bsp/led.h>
#include <c_board_bsp/sys.h>
#include <cmsis_os.h>
#include <stdint.h>

void APP_Setup() {
  BSP_LED_Init();
}

void APP_Loop() {
  static uint8_t hue = 0;

  // 简易 RGB 循环
  BSP_LED_Set_RGB(hue, 255 - hue, (uint8_t)(hue / 2));
  hue++;
  osDelay(20);
}
