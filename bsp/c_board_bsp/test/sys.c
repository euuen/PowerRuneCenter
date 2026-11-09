/*
BSP 系统时间测试（打印运行时间）
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
  static uint32_t last_s = 0;
  uint32_t now_s = (uint32_t)BSP_GetTimestamp_s();
  if (now_s != last_s) {
    last_s = now_s;
    char out_str[32];
    snprintf(out_str, sizeof(out_str), "t=%lu s\n", (unsigned long)now_s);
    BSP_USART_SendStr_BLOCKED(&bsp_usart_1, out_str, 10);
  }
  osDelay(100);
}