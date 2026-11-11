#pragma once

#include "type.h"
#include <stddef.h>
#include <stdint.h>
#include <usart.h>

typedef void (*BSP_USART_RxCallback)(uint8_t *data, uint16_t size);
typedef void (*BSP_USART_TxCpltCallback)(void);

typedef struct {
    UART_HandleTypeDef *huart;

    /* 手动模式：由上层提供 RX 缓冲，收到数据后只回调一次，不自动重启。
     * 非 NULL 时优先于下面的 AddRxCallback 自动模式。 */
    BSP_USART_RxCallback rx_callback;
    BSP_USART_TxCpltCallback tx_cplt_callback;
    uint8_t *rx_buf;
    volatile uint8_t tx_busy;
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

/* 手动 RX/TX 模式：给上面（如 euuen_mod interface 层）提供按缓冲收发的接口 */
BSP_State BSP_USART_SetRxHandler(BSP_USART *bsp_usart, BSP_USART_RxCallback callback);
BSP_State BSP_USART_SetTxHandler(BSP_USART *bsp_usart, BSP_USART_TxCpltCallback callback);
BSP_State BSP_USART_EnableRxHandler(BSP_USART *bsp_usart, uint8_t *buf, uint16_t size);
uint8_t BSP_USART_IsReady(BSP_USART *bsp_usart);
