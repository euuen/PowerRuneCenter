#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// 与 BSP 解耦的 SPI 枚举
typedef enum {
  EM_SPI_1 = 0,
  EM_SPI_NUM,
} EM_SPI_ID;

// EM_SPI_ID -> BSP_SPI_ID 转换表
extern const uint8_t EM_SPI_ConvertTable[EM_SPI_NUM];

/* ---- 驱动 API：只做 BSP 包装 + 防重复初始化（上层 peripheral 使用） ---- */

void EM_SPI_Init(EM_SPI_ID spi);
/* 阻塞式全双工收发；返回 1 成功、0 失败/超时 */
uint8_t EM_SPI_SendReceive(EM_SPI_ID spi, uint8_t *tx, uint8_t *rx,
                           uint16_t length);
/* 按次传入收发缓冲的全双工 DMA（非阻塞启动）；返回 1 成功启动、0 失败 */
uint8_t EM_SPI_SendReceiveDMA(EM_SPI_ID spi, uint8_t *tx, uint8_t *rx,
                              uint16_t length);
uint8_t EM_SPI_IsReady(EM_SPI_ID spi);

#ifdef __cplusplus
}
#endif
