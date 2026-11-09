#include "buzzer.h"
#include <tim.h>

void BSP_Buzzer_Init() {
    MX_TIM4_Init();
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
}

void BSP_Buzzer_Set_Volume(uint16_t voltage) {
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, voltage);
}

void BSP_Buzzer_On() {
    BSP_Buzzer_Set_Volume(200);
}
void BSP_Buzzer_Off() {
    BSP_Buzzer_Set_Volume(0);
}