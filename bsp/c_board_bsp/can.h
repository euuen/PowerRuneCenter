#pragma once
#include "type.h"
#include <can.h>
#include <stddef.h>
#include <stdint.h>

typedef void (*BSP_CAN_RxCallback)(uint32_t can_id, uint8_t *data, uint8_t length);

typedef struct {
    CAN_HandleTypeDef *hcan;
} BSP_CAN;

BSP_State BSP_CAN_1_Init(BSP_CAN *bsp_can);
BSP_State BSP_CAN_2_Init(BSP_CAN *bsp_can);
BSP_State BSP_CAN_1_AddRxCallback(BSP_CAN_RxCallback callback);
BSP_State BSP_CAN_2_AddRxCallback(BSP_CAN_RxCallback callback);
BSP_State BSP_CAN_SendStdFrame(BSP_CAN *bsp_can, uint16_t can_id, uint8_t *data, size_t length, float timeout);
