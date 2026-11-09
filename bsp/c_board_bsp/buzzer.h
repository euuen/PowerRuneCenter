#pragma once

#include <stdint.h>
void BSP_Buzzer_Init();
void BSP_Buzzer_Set_Volume(uint16_t voltage);
void BSP_Buzzer_On();
void BSP_Buzzer_Off();