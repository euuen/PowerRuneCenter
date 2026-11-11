#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <euuen_mod/interface/usart.h>
#include <stddef.h>
#include <stdint.h>

/* 业务层回调（不依赖 BSP / 驱动） */
typedef void (*EM_USART_Ex_RxHandler)(uint8_t *data, uint16_t size);
typedef void (*EM_USART_Ex_TxHandler)(void);

/* ---- 扩展能力公共 API（业务模块调用） ---- */

void EM_USART_Ex_Init(Euuen_USART_ID usart);
void EM_USART_Ex_EnableRx(Euuen_USART_ID usart);
void EM_USART_Ex_AddRxHandler(Euuen_USART_ID usart,
                              EM_USART_Ex_RxHandler handler);
void EM_USART_Ex_AddTxHandler(Euuen_USART_ID usart,
                              EM_USART_Ex_TxHandler handler);
void EM_USART_Ex_SendStr(Euuen_USART_ID usart, const char *str);
size_t EM_USART_Ex_SendData(Euuen_USART_ID usart, const uint8_t *data,
                            size_t length);
uint8_t EM_USART_Ex_IsBusy(Euuen_USART_ID usart);

#ifdef __cplusplus
}
#endif
