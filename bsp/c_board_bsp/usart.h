#pragma once

#include "type.h"
#include <stddef.h>
#include <stdint.h>
#include <usart.h>

typedef void (*BSP_USART_RxCallback)(uint8_t *data, uint16_t size);

typedef struct {
    UART_HandleTypeDef *huart;
} BSP_USART;

void BSP_USART_1_Init(BSP_USART *bsp_usart);
void BSP_USART_2_Init(BSP_USART *bsp_usart);
void BSP_USART_DBUS_Init(BSP_USART *bsp_usart);
void BSP_USART_SendStr_BLOCKED(BSP_USART *bsp_usart, const char *str, uint32_t timeout);
void BSP_USART_SendStr_DMA(BSP_USART *bsp_usart, const char *str);
void BSP_USART_SendData_DMA(BSP_USART *bsp_usart, const uint8_t *data, size_t length);
BSP_State BSP_USART_1_AddRxCallback(BSP_USART_RxCallback callback);
BSP_State BSP_USART_2_AddRxCallback(BSP_USART_RxCallback callback);
BSP_State BSP_USART_DBUS_AddRxCallback(BSP_USART_RxCallback callback);
