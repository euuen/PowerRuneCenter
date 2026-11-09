#include "Looper.h"
#include <c_board_bsp/sys.h>
#include <string.h>

// 注意：受 FreeRTOS 影响（BSP_GetTimestamp_ms 精度为 1ms），
// Looper 最大帧率只有 1000Hz，并且不保证一定为这个帧率，且最多只能塞入 4 个 Loop。

void Looper_Init(Looper *looper) {
  memset(looper, 0, sizeof(Looper));
}

LooperState Looper_Add_Loop(Looper *looper, Looper_LoopCallback callback,
                            float frequency) {
  if (callback == NULL || frequency <= 0.0f || frequency > LOOPER_MAX_FREQ_HZ) {
    return LOOPER_INVALID_FREQ;
  }
  if (looper->count >= LOOPER_MAX_LOOPS) {
    return LOOPER_TOO_MANY_LOOPS;
  }

  looper->loops[looper->count].callback = callback;
  looper->loops[looper->count].frequency = frequency;
  looper->loops[looper->count].last_tick = (uint32_t)BSP_GetTimestamp_ms();
  looper->count++;
  return LOOPER_OK;
}

void Looper_Process(Looper *looper) {
  uint32_t now = (uint32_t)BSP_GetTimestamp_ms();
  for (uint8_t i = 0; i < looper->count; i++) {
    Looper_Loop *loop = &looper->loops[i];
    uint32_t interval_ms = (uint32_t)(1000.0f / loop->frequency);
    if (interval_ms < 1) {
      interval_ms = 1;
    }
    if (now - loop->last_tick >= interval_ms) {
      loop->last_tick = now;
      if (loop->callback != NULL) {
        loop->callback();
      }
    }
  }
}
