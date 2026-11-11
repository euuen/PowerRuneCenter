#include <c_board_bsp/usart.h>
#include <euuen_mod/debug/Logger.h>
#include <euuen_mod/interface/usart.h>
#include <stddef.h>
#include <stdint.h>

/* ---- 平台相关：EUUEN 枚举 -> BSP 句柄索引（换板改这里） ---- */

const uint8_t Euuen_USART_ConvertTable[EUUEN_USART_NUM] = {
    0, /* EUUEN_USART_1    -> BSP_USART_1 */
    1, /* EUUEN_USART_2    -> BSP_USART_2 */
    2, /* EUUEN_USART_DBUS -> BSP_USART_DBUS */
};

/* ---- 驱动状态：每个 Euuen_USART_ID 对应一个 BSP_USART 实例 ---- */

static BSP_USART s_usart[EUUEN_USART_NUM];
static uint8_t s_usart_inited[EUUEN_USART_NUM];
static EM_USART_TxCallback s_tx_cb[EUUEN_USART_NUM];
static EM_USART_RxCallback s_rx_cb[EUUEN_USART_NUM];

/* BSP 回调没有通道参数，因此每个通道使用独立的 trampoline */
static void EM_USART_1_TxTrampoline(void) {
  if (s_tx_cb[EUUEN_USART_1] != NULL) {
    s_tx_cb[EUUEN_USART_1](EUUEN_USART_1);
  }
}

static void EM_USART_2_TxTrampoline(void) {
  if (s_tx_cb[EUUEN_USART_2] != NULL) {
    s_tx_cb[EUUEN_USART_2](EUUEN_USART_2);
  }
}

static void EM_USART_DBUS_TxTrampoline(void) {
  if (s_tx_cb[EUUEN_USART_DBUS] != NULL) {
    s_tx_cb[EUUEN_USART_DBUS](EUUEN_USART_DBUS);
  }
}

static void EM_USART_1_RxTrampoline(uint8_t *data, uint16_t size) {
  if (s_rx_cb[EUUEN_USART_1] != NULL) {
    s_rx_cb[EUUEN_USART_1](EUUEN_USART_1, data, (size_t)size);
  }
}

static void EM_USART_2_RxTrampoline(uint8_t *data, uint16_t size) {
  if (s_rx_cb[EUUEN_USART_2] != NULL) {
    s_rx_cb[EUUEN_USART_2](EUUEN_USART_2, data, (size_t)size);
  }
}

static void EM_USART_DBUS_RxTrampoline(uint8_t *data, uint16_t size) {
  if (s_rx_cb[EUUEN_USART_DBUS] != NULL) {
    s_rx_cb[EUUEN_USART_DBUS](EUUEN_USART_DBUS, data, (size_t)size);
  }
}

static BSP_USART *EM_USART_Get(Euuen_USART_ID usart) {
  if (usart < 0 || usart >= EUUEN_USART_NUM) {
    return NULL;
  }
  if (!s_usart_inited[usart]) {
    switch (usart) {
    case EUUEN_USART_1:
      BSP_USART_1_Init(&s_usart[usart]);
      break;
    case EUUEN_USART_2:
      BSP_USART_2_Init(&s_usart[usart]);
      break;
    case EUUEN_USART_DBUS:
      BSP_USART_DBUS_Init(&s_usart[usart]);
      break;
    default:
      return NULL;
    }
    s_usart_inited[usart] = 1;
  }
  return &s_usart[usart];
}

/* ---- 驱动 API ---- */

void EM_USART_Init(Euuen_USART_ID usart) {
  if (usart < 0 || usart >= EUUEN_USART_NUM) {
    return;
  }
  if (s_usart_inited[usart]) {
    LOG_WARN("[EM_USART_Init] usart %d already initialized, skipped",
             (int)usart);
    return;
  }
  (void)EM_USART_Get(usart);
}

void EM_USART_SendData_DMA(Euuen_USART_ID usart, const uint8_t *data,
                           size_t length) {
  if (data == NULL || length == 0) {
    return;
  }
  BSP_USART *bsp_usart = EM_USART_Get(usart);
  if (bsp_usart == NULL) {
    return;
  }
  BSP_USART_SendData_DMA(bsp_usart, data, length);
}

void EM_USART_SetTxHandler(Euuen_USART_ID usart, EM_USART_TxCallback callback) {
  if (usart < 0 || usart >= EUUEN_USART_NUM || callback == NULL) {
    return;
  }
  BSP_USART *bsp_usart = EM_USART_Get(usart);
  if (bsp_usart == NULL) {
    return;
  }
  s_tx_cb[usart] = callback;
  switch (usart) {
  case EUUEN_USART_1:
    BSP_USART_SetTxHandler(bsp_usart, EM_USART_1_TxTrampoline);
    break;
  case EUUEN_USART_2:
    BSP_USART_SetTxHandler(bsp_usart, EM_USART_2_TxTrampoline);
    break;
  case EUUEN_USART_DBUS:
    BSP_USART_SetTxHandler(bsp_usart, EM_USART_DBUS_TxTrampoline);
    break;
  default:
    break;
  }
}

void EM_USART_SetRxHandler(Euuen_USART_ID usart, EM_USART_RxCallback callback) {
  if (usart < 0 || usart >= EUUEN_USART_NUM || callback == NULL) {
    return;
  }
  BSP_USART *bsp_usart = EM_USART_Get(usart);
  if (bsp_usart == NULL) {
    return;
  }
  s_rx_cb[usart] = callback;
  switch (usart) {
  case EUUEN_USART_1:
    BSP_USART_SetRxHandler(bsp_usart, EM_USART_1_RxTrampoline);
    break;
  case EUUEN_USART_2:
    BSP_USART_SetRxHandler(bsp_usart, EM_USART_2_RxTrampoline);
    break;
  case EUUEN_USART_DBUS:
    BSP_USART_SetRxHandler(bsp_usart, EM_USART_DBUS_RxTrampoline);
    break;
  default:
    break;
  }
}

void EM_USART_EnableRxHandler(Euuen_USART_ID usart, uint8_t *buf,
                              uint16_t size) {
  if (buf == NULL || size == 0) {
    return;
  }
  BSP_USART *bsp_usart = EM_USART_Get(usart);
  if (bsp_usart == NULL) {
    return;
  }
  BSP_USART_EnableRxHandler(bsp_usart, buf, size);
}
