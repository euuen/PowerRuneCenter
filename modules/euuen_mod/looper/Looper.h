#pragma once

#include <stdint.h>

// 注意：受 FreeRTOS 影响（osDelay 精度为 ms），
// Looper 最大帧率只有 1000Hz，且最多只能塞入 3 个 Loop。
#define LOOPER_MAX_LOOPS 3
#define LOOPER_MAX_FREQ_HZ 1000

typedef void (*Looper_LoopCallback)(void);

typedef enum {
    LOOPER_OK,
    LOOPER_ERROR,
    LOOPER_TOO_MANY_LOOPS,
    LOOPER_INVALID_FREQ,
} LooperState;

typedef struct {
    Looper_LoopCallback callback;
    float frequency;
    uint32_t last_tick;
} Looper_Loop;

typedef struct {
    Looper_Loop loops[LOOPER_MAX_LOOPS];
    uint8_t count;
} Looper;

void Looper_Init(Looper *looper);
LooperState Looper_Add_Loop(Looper *looper, Looper_LoopCallback callback, float frequency);
void Looper_Process(Looper *looper);
