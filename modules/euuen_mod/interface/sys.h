#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// 临界区
void EM_EnterCritical(void);
void EM_ExitCritical(void);
uint32_t EM_EnterCriticalFromISR(void);
void EM_ExitCriticalFromISR(uint32_t handle);

// 时间
float EM_GetTimestamp_s(void);
float EM_GetTimestamp_ms(void);
uint64_t EM_GetTimestamp_us(void);
void EM_Delay_ms(uint32_t ms);
void EM_Delay_us(uint32_t us);

// 互斥量（FreeRTOS 互斥量包装；句柄对上层不透明）
typedef void *EM_MutexId;
EM_MutexId EM_MutexCreate(void);
void EM_MutexTake(EM_MutexId mutex); /* 阻塞直到拿到 */
void EM_MutexGive(EM_MutexId mutex);

// 线程
typedef enum {
  EUUEN_THREAD_PRIORITY_IDLE = -3,
  EUUEN_THREAD_PRIORITY_LOW = -2,
  EUUEN_THREAD_PRIORITY_BELOW_NORMAL = -1,
  EUUEN_THREAD_PRIORITY_NORMAL = 0,
  EUUEN_THREAD_PRIORITY_ABOVE_NORMAL = 1,
  EUUEN_THREAD_PRIORITY_HIGH = 2,
  EUUEN_THREAD_PRIORITY_REALTIME = 3,
} Euuen_ThreadPriority;

typedef void (*Euuen_ThreadFunction_t)(void const *argument);

void EM_CreateThread(const char *name, Euuen_ThreadFunction_t function,
                     Euuen_ThreadPriority priority, uint32_t stack_size,
                     void *argument);

#ifdef __cplusplus
}
#endif
