#pragma once

#include "type.h"
#include <spi.h>
#include <stdint.h>

#define BSP_SPI_TIMEOUT_MS 10 /* 阻塞收发超时 */

/* SPI 实例：调用方持有，Init 时绑定到具体硬件 SPI */
typedef struct {
    SPI_HandleTypeDef *hspi;
    volatile uint8_t busy; /* DMA 传输中 */
} BSP_SPI;

BSP_State BSP_SPI_1_Init(BSP_SPI *bsp_spi);

/* 阻塞式全双工收发（HAL_SPI_TransmitReceive） */
BSP_State BSP_SPI_SendReceive(BSP_SPI *bsp_spi, uint8_t *tx_buf,
                              uint8_t *rx_buf, uint16_t length);

/* 按次传入收发缓冲的全双工 DMA（非阻塞启动） */
BSP_State BSP_SPI_SendReceiveDMA(BSP_SPI *bsp_spi, uint8_t *tx_buf,
                                 uint8_t *rx_buf, uint16_t length);

uint8_t BSP_SPI_IsReady(BSP_SPI *bsp_spi);
