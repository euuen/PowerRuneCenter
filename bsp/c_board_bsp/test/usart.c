/*
BSP USART 测试（周期打印）
代码测试完毕，没有发现问题。
*/

#include <c_board_bsp/sys.h>
#include <c_board_bsp/usart.h>
#include <cmsis_os.h>
#include <stdint.h>
#include <stdio.h>

BSP_USART bsp_usart_1;

void APP_Setup() {
  BSP_USART_1_Init(&bsp_usart_1);
}

void APP_Loop() {
  char out_str[32];
  snprintf(out_str, sizeof(out_str), "usart ok %lu\n", (unsigned long)BSP_GetTimestamp_ms());
  BSP_USART_SendStr_BLOCKED(&bsp_usart_1, out_str, 10);
  osDelay(100);
}