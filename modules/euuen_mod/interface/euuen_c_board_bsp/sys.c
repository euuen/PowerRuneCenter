#include <c_board_bsp/sys.h>
#include <cmsis_os.h>
#include <euuen_mod/interface/sys.h>
#include <stdint.h>

/* ---- 基本工具（与 BSP/USART/CAN 无关） ---- */

void EM_EnterCritical(void) { taskENTER_CRITICAL(); }

void EM_ExitCritical(void) { taskEXIT_CRITICAL(); }

uint32_t EM_EnterCriticalFromISR(void) {
  return taskENTER_CRITICAL_FROM_ISR();
}

void EM_ExitCriticalFromISR(uint32_t handle) {
  taskEXIT_CRITICAL_FROM_ISR(handle);
}

float EM_GetTimestamp_s(void) { return BSP_GetTimestamp_s(); }
float EM_GetTimestamp_ms(void) { return BSP_GetTimestamp_ms(); }
uint64_t EM_GetTimestamp_us(void) { return BSP_GetTimestamp_us(); }

void EM_Delay_ms(uint32_t ms) { osDelay(ms); }

void EM_Delay_us(uint32_t us) { BSP_Delay_us((float)us); }

/* ---- 互斥量（平台相关：封装 FreeRTOS 互斥量，句柄对上层不透明） ---- */

EM_MutexId EM_MutexCreate(void) {
  return (EM_MutexId)xSemaphoreCreateMutex();
}

void EM_MutexTake(EM_MutexId mutex) {
  if (mutex == NULL) {
    return;
  }
  xSemaphoreTake((SemaphoreHandle_t)mutex, portMAX_DELAY);
}

void EM_MutexGive(EM_MutexId mutex) {
  if (mutex == NULL) {
    return;
  }
  xSemaphoreGive((SemaphoreHandle_t)mutex);
}

/* ---- 线程 ---- */

void EM_CreateThread(const char *name, Euuen_ThreadFunction_t function,
                     Euuen_ThreadPriority priority, uint32_t stack_size,
                     void *argument) {
  static osThreadDef_t thread_def;
  thread_def.name = (char *)name;
  thread_def.pthread = (os_pthread)function;
  thread_def.tpriority = (osPriority)priority;
  thread_def.instances = 1;
  thread_def.stacksize = stack_size;
  osThreadCreate(&thread_def, argument);
}
