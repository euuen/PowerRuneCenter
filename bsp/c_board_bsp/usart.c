#include "usart.h"
#include "parameter.h"
#include <stdint.h>
#include <string.h>
#include <usart.h>

#define USART_1_INITED_MASK (0x1 << 0)
#define USART_3_INITED_MASK (0x1 << 1)
#define USART_6_INITED_MASK (0x1 << 2)

static uint8_t check_usart_inited = 0;

/* 本板映射：BSP_USART_1 -> huart6，BSP_USART_2 -> huart1，
 * BSP_USART_DBUS -> huart3。 */
/* 由 Init 绑定，HAL 回调里用来找到对应实例（手动模式） */
static BSP_USART *bsp_usart_1_instance = NULL;
static BSP_USART *bsp_usart_2_instance = NULL;
static BSP_USART *bsp_usart_dbus_instance = NULL;

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

/* 自动模式：重新挂上 BSP 内部缓冲 */
static void StartAutoRx(UART_HandleTypeDef *huart, uint8_t *buf) {
  HAL_UARTEx_ReceiveToIdle_DMA(huart, buf, BSP_USART_RX_BUF_SIZE);
  __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);
}

void BSP_USART_1_Init(BSP_USART *bsp_usart) {
  if (!(check_usart_inited & USART_6_INITED_MASK)) {
    MX_USART6_UART_Init();
    check_usart_inited |= USART_6_INITED_MASK;
  }
  bsp_usart->huart = &huart6;
  bsp_usart->tx_busy = 0;
  bsp_usart_1_instance = bsp_usart;
}

void BSP_USART_2_Init(BSP_USART *bsp_usart) {
  if (!(check_usart_inited & USART_1_INITED_MASK)) {
    MX_USART1_UART_Init();
    check_usart_inited |= USART_1_INITED_MASK;
  }
  bsp_usart->huart = &huart1;
  bsp_usart->tx_busy = 0;
  bsp_usart_2_instance = bsp_usart;
}

void BSP_USART_DBUS_Init(BSP_USART *bsp_usart) {
  if (!(check_usart_inited & USART_3_INITED_MASK)) {
    MX_USART3_UART_Init();
    check_usart_inited |= USART_3_INITED_MASK;
  }
  bsp_usart->huart = &huart3;
  bsp_usart->tx_busy = 0;
  bsp_usart_dbus_instance = bsp_usart;
}

void BSP_USART_SendStr_BLOCKED(BSP_USART *bsp_usart, const char *str,
                               uint32_t timeout) {
  if (bsp_usart == NULL || bsp_usart->huart == NULL) {
    return;
  }
  HAL_UART_Transmit(bsp_usart->huart, (uint8_t *)str, strlen(str), timeout);
}

void BSP_USART_SendStr_DMA(BSP_USART *bsp_usart, const char *str) {
  if (bsp_usart == NULL || bsp_usart->huart == NULL) {
    return;
  }
  bsp_usart->tx_busy = 1;
  HAL_UART_Transmit_DMA(bsp_usart->huart, (uint8_t *)str, strlen(str));
}

void BSP_USART_SendData_DMA(BSP_USART *bsp_usart, const uint8_t *data,
                            size_t length) {
  if (bsp_usart == NULL || bsp_usart->huart == NULL) {
    return;
  }
  bsp_usart->tx_busy = 1;
  HAL_UART_Transmit_DMA(bsp_usart->huart, data, length);
}

/* ---- 手动模式 API ---- */

BSP_State BSP_USART_SetRxHandler(BSP_USART *bsp_usart,
                                 BSP_USART_RxCallback callback) {
  if (bsp_usart == NULL || callback == NULL) {
    return BSP_ERROR;
  }
  bsp_usart->rx_callback = callback;
  return BSP_OK;
}

BSP_State BSP_USART_SetTxHandler(BSP_USART *bsp_usart,
                                 BSP_USART_TxCpltCallback callback) {
  if (bsp_usart == NULL || callback == NULL) {
    return BSP_ERROR;
  }
  bsp_usart->tx_cplt_callback = callback;
  return BSP_OK;
}

BSP_State BSP_USART_EnableRxHandler(BSP_USART *bsp_usart, uint8_t *buf,
                                    uint16_t size) {
  if (bsp_usart == NULL || bsp_usart->huart == NULL || buf == NULL ||
      size == 0) {
    return BSP_ERROR;
  }
  bsp_usart->rx_buf = buf;

  /* 若自动模式还占着该串口，先中止再交给手动模式 */
  HAL_StatusTypeDef state =
      HAL_UARTEx_ReceiveToIdle_DMA(bsp_usart->huart, buf, size);
  if (state == HAL_BUSY) {
    HAL_UART_AbortReceive(bsp_usart->huart);
    state = HAL_UARTEx_ReceiveToIdle_DMA(bsp_usart->huart, buf, size);
  }
  if (state != HAL_OK) {
    return BSP_ERROR;
  }
  __HAL_DMA_DISABLE_IT(bsp_usart->huart->hdmarx, DMA_IT_HT);
  return BSP_OK;
}

uint8_t BSP_USART_IsReady(BSP_USART *bsp_usart) {
  if (bsp_usart == NULL || bsp_usart->huart == NULL) {
    return 1;
  }
  return (bsp_usart->tx_busy == 0);
}

/* ---- 自动模式 API（当前新框架模块使用） ---- */

BSP_State BSP_USART_1_AddRxCallback(BSP_USART_RxCallback callback) {
  for (uint8_t i = 0; i < BSP_USART_1_MAX_RX_CALLBACKS; i++) {
    if (usart1_callbacks[i] == NULL) {
      usart1_callbacks[i] = callback;
      if (bsp_usart_1_instance == NULL ||
          bsp_usart_1_instance->rx_callback == NULL) {
        StartAutoRx(&huart6, usart1_rx_buf);
      }
      return BSP_OK;
    }
  }
  return BSP_ERROR;
}

BSP_State BSP_USART_2_AddRxCallback(BSP_USART_RxCallback callback) {
  for (uint8_t i = 0; i < BSP_USART_2_MAX_RX_CALLBACKS; i++) {
    if (usart2_callbacks[i] == NULL) {
      usart2_callbacks[i] = callback;
      if (bsp_usart_2_instance == NULL ||
          bsp_usart_2_instance->rx_callback == NULL) {
        StartAutoRx(&huart1, usart2_rx_buf);
      }
      return BSP_OK;
    }
  }
  return BSP_ERROR;
}

BSP_State BSP_USART_DBUS_AddRxCallback(BSP_USART_RxCallback callback) {
  for (uint8_t i = 0; i < BSP_USART_DBUS_MAX_RX_CALLBACKS; i++) {
    if (dbus_callbacks[i] == NULL) {
      dbus_callbacks[i] = callback;
      if (bsp_usart_dbus_instance == NULL ||
          bsp_usart_dbus_instance->rx_callback == NULL) {
        StartAutoRx(&huart3, dbus_rx_buf);
      }
      return BSP_OK;
    }
  }
  return BSP_ERROR;
}

/* ---- HAL 回调：手动模式优先，否则走自动模式 ---- */

static void HandleRxEvent(BSP_USART *instance, UART_HandleTypeDef *huart,
                          BSP_USART_RxCallback *callbacks,
                          uint8_t max_callbacks, uint8_t *auto_buf,
                          uint16_t size) {
  if (instance != NULL && instance->rx_callback != NULL) {
    /* 手动模式：不自动重启，由上层（块泵）重新 EnableRxHandler */
    instance->rx_callback(instance->rx_buf, size);
    return;
  }

  DispatchCallbacks(callbacks, max_callbacks, auto_buf, size);
  StartAutoRx(huart, auto_buf);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size) {
  if (huart == &huart1) {
    HandleRxEvent(bsp_usart_2_instance, huart, usart2_callbacks,
                  BSP_USART_2_MAX_RX_CALLBACKS, usart2_rx_buf, size);
  } else if (huart == &huart6) {
    HandleRxEvent(bsp_usart_1_instance, huart, usart1_callbacks,
                  BSP_USART_1_MAX_RX_CALLBACKS, usart1_rx_buf, size);
  } else if (huart == &huart3) {
    HandleRxEvent(bsp_usart_dbus_instance, huart, dbus_callbacks,
                  BSP_USART_DBUS_MAX_RX_CALLBACKS, dbus_rx_buf, size);
  }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
  BSP_USART *instance = NULL;
  if (huart == &huart1) {
    instance = bsp_usart_2_instance;
  } else if (huart == &huart6) {
    instance = bsp_usart_1_instance;
  } else if (huart == &huart3) {
    instance = bsp_usart_dbus_instance;
  }

  if (instance == NULL) {
    return;
  }
  instance->tx_busy = 0;
  if (instance->tx_cplt_callback != NULL) {
    instance->tx_cplt_callback();
  }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
  if (huart == &huart1) {
    if (bsp_usart_2_instance != NULL &&
        bsp_usart_2_instance->rx_callback != NULL) {
      /* size=0 通知上层重新 arm */
      bsp_usart_2_instance->rx_callback(bsp_usart_2_instance->rx_buf, 0);
    } else {
      StartAutoRx(&huart1, usart2_rx_buf);
    }
  } else if (huart == &huart6) {
    if (bsp_usart_1_instance != NULL &&
        bsp_usart_1_instance->rx_callback != NULL) {
      bsp_usart_1_instance->rx_callback(bsp_usart_1_instance->rx_buf, 0);
    } else {
      StartAutoRx(&huart6, usart1_rx_buf);
    }
  } else if (huart == &huart3) {
    if (bsp_usart_dbus_instance != NULL &&
        bsp_usart_dbus_instance->rx_callback != NULL) {
      bsp_usart_dbus_instance->rx_callback(bsp_usart_dbus_instance->rx_buf, 0);
    } else {
      StartAutoRx(&huart3, dbus_rx_buf);
    }
  }
}
