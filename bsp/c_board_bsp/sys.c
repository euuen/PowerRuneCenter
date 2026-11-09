#include "sys.h"
#include <cmsis_os.h>
#include <dma.h>
#include <gpio.h>

static uint32_t CPU_FREQ_Hz;
static uint32_t CPU_FREQ_Hz_ms;
static uint32_t CPU_FREQ_Hz_us;

static volatile uint32_t CYCCNT_RountCount = 0;
static volatile uint32_t CYCCNT_LAST = 0;

static BSP_Time_t SysTime;

static void StartRootTask(void const *argument) {
  APP_Setup();
  for (;;) {
    APP_Loop();
  }
}

__attribute__((weak)) void APP_Setup() {}

__attribute__((weak)) void APP_Loop() { osDelay(1); }

/**
 * @brief 原子读取当前64位DWT计数值(临界区内调用)
 *        单次读取CYCCNT,完成溢出检测
 */
static uint64_t BSP_Time_GetCurrent64(void) {
  uint32_t cnt_now = DWT->CYCCNT;
  if (cnt_now < CYCCNT_LAST)
    CYCCNT_RountCount++;
  CYCCNT_LAST = cnt_now;
  return (uint64_t)CYCCNT_RountCount * (uint64_t)UINT32_MAX + (uint64_t)cnt_now;
}

void BSP_Time_Init(uint32_t CPU_Freq_mHz) {
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = (uint32_t)0u;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  CPU_FREQ_Hz = CPU_Freq_mHz * 1000000;
  CPU_FREQ_Hz_ms = CPU_FREQ_Hz / 1000;
  CPU_FREQ_Hz_us = CPU_FREQ_Hz / 1000000;
  CYCCNT_RountCount = 0;
  CYCCNT_LAST = DWT->CYCCNT;

  osThreadDef(BSP_Time_Task, Start_BSP_Time_Task, osPriorityIdle, 0, 128);
  osThreadCreate(osThread(BSP_Time_Task), NULL);
}

void BSP_Time_Update(void) {
  taskENTER_CRITICAL();
  uint64_t cnt64 = BSP_Time_GetCurrent64();

  uint32_t sec = cnt64 / CPU_FREQ_Hz;
  uint64_t remain = cnt64 - (uint64_t)sec * CPU_FREQ_Hz;
  uint32_t ms = remain / CPU_FREQ_Hz_ms;
  remain -= (uint64_t)ms * CPU_FREQ_Hz_ms;
  uint32_t us = remain / CPU_FREQ_Hz_us;

  SysTime.s = sec;
  SysTime.ms = ms;
  SysTime.us = us;
  taskEXIT_CRITICAL();
}

void BSP_Time_Update_ISR(void) {
  UBaseType_t uxSavedStatusRegister = taskENTER_CRITICAL_FROM_ISR();
  uint64_t cnt64 = BSP_Time_GetCurrent64();

  uint32_t sec = cnt64 / CPU_FREQ_Hz;
  uint64_t remain = cnt64 - (uint64_t)sec * CPU_FREQ_Hz;
  uint32_t ms = remain / CPU_FREQ_Hz_ms;
  remain -= (uint64_t)ms * CPU_FREQ_Hz_ms;
  uint32_t us = remain / CPU_FREQ_Hz_us;

  SysTime.s = sec;
  SysTime.ms = ms;
  SysTime.us = us;
  taskEXIT_CRITICAL_FROM_ISR(uxSavedStatusRegister);
}


float BSP_GetTimestamp_s(void) {
  BSP_Time_Update();
  return SysTime.s + SysTime.ms * 0.001f + SysTime.us * 0.000001f;
}

float BSP_GetTimestamp_s_ISR(void) {
  BSP_Time_Update_ISR();
  return SysTime.s + SysTime.ms * 0.001f + SysTime.us * 0.000001f;
}


float BSP_GetTimestamp_ms(void) {
  BSP_Time_Update();
  return SysTime.s * 1000.0f + SysTime.ms + SysTime.us * 0.001f;
}

float BSP_GetTimestamp_ms_ISR(void) {
  BSP_Time_Update_ISR();
  return SysTime.s * 1000.0f + SysTime.ms + SysTime.us * 0.001f;
}

uint64_t BSP_GetTimestamp_us(void) {
  BSP_Time_Update();
  return (uint64_t)SysTime.s * 1000000 + SysTime.ms * 1000 + SysTime.us;
}

uint64_t BSP_GetTimestamp_us_ISR(void) {
  BSP_Time_Update_ISR();
  return (uint64_t)SysTime.s * 1000000 + SysTime.ms * 1000 + SysTime.us;
}

void BSP_Delay_s(float Delay) {
  uint32_t tickstart = DWT->CYCCNT;
  while ((DWT->CYCCNT - tickstart) < Delay * (float)CPU_FREQ_Hz)
    ;
}

void BSP_Delay_ms(float Delay) {
  uint32_t tickstart = DWT->CYCCNT;
  while ((DWT->CYCCNT - tickstart) < Delay * (float)CPU_FREQ_Hz_ms)
    ;
}

void BSP_Delay_us(float Delay) {
  uint32_t tickstart = DWT->CYCCNT;
  while ((DWT->CYCCNT - tickstart) < Delay * (float)CPU_FREQ_Hz_us)
    ;
}

void Start_BSP_Time_Task(void const *argument) {
  for (;;) {
    BSP_Time_Update();
    osDelay(1);
  }
}

__attribute__((weak)) void BSP_System_Setup() {
  MX_GPIO_Init();
  MX_DMA_Init();

  osThreadDef(rootTask, StartRootTask, osPriorityNormal, 0, 1024);
  osThreadCreate(osThread(rootTask), NULL);

  BSP_Time_Init(168);
  osKernelStart();
}
