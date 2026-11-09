/*
Looper 模块测试（定时调度回调）
代码测试完毕，没有发现问题。
*/

#include <c_board_bsp/sys.h>
#include <c_board_bsp/usart.h>
#include <cmsis_os.h>
#include <euuen_mod/debug/Logger.h>
#include <euuen_mod/looper/Looper.h>
#include <stdint.h>
#include <stdio.h>

BSP_USART bsp_usart_1;
Logger logger;
Looper looper;

static uint32_t s_count_10hz = 0;
static uint32_t s_count_1hz = 0;

static void Loop_10Hz(void) { s_count_10hz++; }

static void Loop_1Hz(void) { s_count_1hz++; }

void APP_Setup() {
  BSP_USART_1_Init(&bsp_usart_1);
  Logger_Init(&logger, &bsp_usart_1);

  Looper_Init(&looper);
  Looper_Add_Loop(&looper, Loop_10Hz, 10.0f);
  Looper_Add_Loop(&looper, Loop_1Hz, 1.0f);
  Logger_Log(&logger, "looper inited\n");
}

void APP_Loop() {
  Looper_Process(&looper);

  char out_str[64];
  snprintf(out_str, sizeof(out_str), "10hz=%u 1hz=%u\n", (unsigned)s_count_10hz, (unsigned)s_count_1hz);
  Logger_Log(&logger, out_str);
  osDelay(10);
}
