#include "led.h"
#include <tim.h>

void BSP_LED_Init() {
    MX_TIM5_Init();
    HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_3);
}

void BSP_LED_Set_RGB(uint8_t R, uint8_t G, uint8_t B) {
    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, B);
    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_2, G);
    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_3, R);
}