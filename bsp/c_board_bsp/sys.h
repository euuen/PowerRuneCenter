#pragma once

#include "stdint.h"

typedef struct {
    uint32_t s;
    uint16_t ms;
    uint16_t us;
} BSP_Time_t;

// public
void APP_Setup();
void APP_Loop();
void BSP_System_Setup();
float BSP_GetTimestamp_s(void);
float BSP_GetTimestamp_ms(void);
uint64_t BSP_GetTimestamp_us(void);
float BSP_GetTimestamp_s_ISR(void);
float BSP_GetTimestamp_ms_ISR(void);
uint64_t BSP_GetTimestamp_us_ISR(void);
void BSP_Delay_s(float Delay);
void BSP_Delay_ms(float Delay);
void BSP_Delay_us(float Delay);

//private
void BSP_Time_Init(uint32_t CPU_Freq_mHz);
void BSP_Time_Update(void);
void BSP_Time_Update_ISR(void);
float BSP_GetTimestamp_ms_ISR(void);
void Start_BSP_Time_Task(void const *argument);
