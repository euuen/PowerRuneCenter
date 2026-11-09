#include "key.h"
#include <gpio.h>
#include <stdint.h>

uint8_t BSP_Key_Is_Pressed() {
  // GPIO的初始化，我觉得搞起来太麻烦就直接调用cubemx的函数了，具体见bsp_sys.c
  if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET)
    return 1;
  else
    return 0;
}