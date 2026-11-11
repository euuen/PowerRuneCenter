#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

// 与 BSP 解耦的外设枚举
typedef enum {
  EUUEN_USART_1 = 0,
  EUUEN_USART_2,
  EUUEN_USART_DBUS,
  EUUEN_USART_NUM,
} Euuen_USART_ID;

// EUUEN 枚举 -> BSP 枚举转换表（值仅作句柄索引，具体含义见 interface 实现）
extern const uint8_t Euuen_USART_ConvertTable[EUUEN_USART_NUM];

/* 驱动回调：参数用 EUUEN 枚举，由 interface 负责 BSP id <-> EUUEN 枚举的映射 */
typedef void (*EM_USART_TxCallback)(Euuen_USART_ID usart);
typedef void (*EM_USART_RxCallback)(Euuen_USART_ID usart, uint8_t *data,
                                    size_t size);

/* ---- 驱动 API：只做 BSP 包装 + 防重复初始化（上层 peripheral 使用） ---- */

void EM_USART_Init(Euuen_USART_ID usart);
void EM_USART_SendData_DMA(Euuen_USART_ID usart, const uint8_t *data,
                           size_t length);
void EM_USART_SetTxHandler(Euuen_USART_ID usart, EM_USART_TxCallback callback);
void EM_USART_SetRxHandler(Euuen_USART_ID usart, EM_USART_RxCallback callback);
void EM_USART_EnableRxHandler(Euuen_USART_ID usart, uint8_t *buf, uint16_t size);

#ifdef __cplusplus
}
#endif
