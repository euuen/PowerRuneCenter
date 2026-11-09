#include "usart.h"
#include "parameter.h"
#include <stdint.h>
#include <string.h>
#include <usart.h>

#define USART_1_INITED_MASK (0x1 << 0)
#define USART_3_INITED_MASK (0x1 << 1)
#define USART_6_INITED_MASK (0x1 << 2)

static uint8_t check_usart_inited = 0;

static BSP_USART_RxCallback usart1_callbacks[BSP_USART_1_MAX_RX_CALLBACKS] = {
    0};
static BSP_USART_RxCallback usart2_callbacks[BSP_USART_2_MAX_RX_CALLBACKS] = {
    0};
static BSP_USART_RxCallback dbus_callbacks[BSP_USART_DBUS_MAX_RX_CALLBACKS] = {
    0};

static uint8_t usart1_rx_buf[BSP_USART_RX_BUF_SIZE];
static uint8_t usart2_rx_buf[BSP_USART_RX_BUF_SIZE];
static uint8_t dbus_rx_buf[BSP_USART_RX_BUF_SIZE];

static void DispatchCallbacks(BSP_USART_RxCallback *callbacks,
                              uint8_t max_callbacks, uint8_t *data,
                              uint16_t size) {
  for (uint8_t i = 0; i < max_callbacks; i++) {
    if (callbacks[i] != NULL) {
      callbacks[i](data, size);
    }
  }
}

void BSP_USART_1_Init(BSP_USART *bsp_usart) {
  if (!(check_usart_inited & USART_6_INITED_MASK)) {
    MX_USART6_UART_Init();
    check_usart_inited |= USART_6_INITED_MASK;
  }
  bsp_usart->huart = &huart6;
}

void BSP_USART_2_Init(BSP_USART *bsp_usart) {
  if (!(check_usart_inited & USART_1_INITED_MASK)) {
    MX_USART1_UART_Init();
    check_usart_inited |= USART_1_INITED_MASK;
  }
  bsp_usart->huart = &huart1;
}

void BSP_USART_DBUS_Init(BSP_USART *bsp_usart) {
  if (!(check_usart_inited & USART_3_INITED_MASK)) {
    MX_USART3_UART_Init();
    check_usart_inited |= USART_3_INITED_MASK;
  }
  bsp_usart->huart = &huart3;
}

void BSP_USART_SendStr_BLOCKED(BSP_USART *bsp_usart, const char *str,
                               uint32_t timeout) {
  HAL_UART_Transmit(bsp_usart->huart, (uint8_t *)str, strlen(str), timeout);
}

void BSP_USART_SendStr_DMA(BSP_USART *bsp_usart, const char *str) {
  HAL_UART_Transmit_DMA(bsp_usart->huart, (uint8_t *)str, strlen(str));
}

void BSP_USART_SendData_DMA(BSP_USART *bsp_usart, const uint8_t *data,
                            size_t length) {
  HAL_UART_Transmit_DMA(bsp_usart->huart, data, length);
}

BSP_State BSP_USART_1_AddRxCallback(BSP_USART_RxCallback callback) {
  for (uint8_t i = 0; i < BSP_USART_1_MAX_RX_CALLBACKS; i++) {
    if (usart1_callbacks[i] == NULL) {
      usart1_callbacks[i] = callback;
      HAL_UARTEx_ReceiveToIdle_DMA(&huart1, usart1_rx_buf,
                                   BSP_USART_RX_BUF_SIZE);
      __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
      return BSP_OK;
    }
  }
  return BSP_ERROR;
}

BSP_State BSP_USART_2_AddRxCallback(BSP_USART_RxCallback callback) {
  for (uint8_t i = 0; i < BSP_USART_2_MAX_RX_CALLBACKS; i++) {
    if (usart2_callbacks[i] == NULL) {
      usart2_callbacks[i] = callback;
      HAL_UARTEx_ReceiveToIdle_DMA(&huart6, usart2_rx_buf,
                                   BSP_USART_RX_BUF_SIZE);
      __HAL_DMA_DISABLE_IT(huart6.hdmarx, DMA_IT_HT);
      return BSP_OK;
    }
  }
  return BSP_ERROR;
}

BSP_State BSP_USART_DBUS_AddRxCallback(BSP_USART_RxCallback callback) {
  for (uint8_t i = 0; i < BSP_USART_DBUS_MAX_RX_CALLBACKS; i++) {
    if (dbus_callbacks[i] == NULL) {
      dbus_callbacks[i] = callback;
      HAL_UARTEx_ReceiveToIdle_DMA(&huart3, dbus_rx_buf, BSP_USART_RX_BUF_SIZE);
      __HAL_DMA_DISABLE_IT(huart3.hdmarx, DMA_IT_HT);
      return BSP_OK;
    }
  }
  return BSP_ERROR;
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size) {
  if (huart == &huart1) {
    DispatchCallbacks(usart1_callbacks, BSP_USART_1_MAX_RX_CALLBACKS,
                      usart1_rx_buf, size);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, usart1_rx_buf, BSP_USART_RX_BUF_SIZE);
    __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
  } else if (huart == &huart6) {
    DispatchCallbacks(usart2_callbacks, BSP_USART_2_MAX_RX_CALLBACKS,
                      usart2_rx_buf, size);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart6, usart2_rx_buf, BSP_USART_RX_BUF_SIZE);
    __HAL_DMA_DISABLE_IT(huart6.hdmarx, DMA_IT_HT);
  } else if (huart == &huart3) {
    DispatchCallbacks(dbus_callbacks, BSP_USART_DBUS_MAX_RX_CALLBACKS,
                      dbus_rx_buf, size);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart3, dbus_rx_buf, BSP_USART_RX_BUF_SIZE);
    __HAL_DMA_DISABLE_IT(huart3.hdmarx, DMA_IT_HT);
  }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
  if (huart == &huart1) {
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, usart1_rx_buf, BSP_USART_RX_BUF_SIZE);
    __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
  } else if (huart == &huart6) {
    HAL_UARTEx_ReceiveToIdle_DMA(&huart6, usart2_rx_buf, BSP_USART_RX_BUF_SIZE);
    __HAL_DMA_DISABLE_IT(huart6.hdmarx, DMA_IT_HT);
  } else if (huart == &huart3) {
    HAL_UARTEx_ReceiveToIdle_DMA(&huart3, dbus_rx_buf, BSP_USART_RX_BUF_SIZE);
    __HAL_DMA_DISABLE_IT(huart3.hdmarx, DMA_IT_HT);
  }
}
