#include <c_board_bsp/can.h>
#include <euuen_mod/debug/Logger.h>
#include <euuen_mod/interface/can.h>
#include <stddef.h>
#include <stdint.h>

/* ---- 平台相关：EUUEN 枚举 -> BSP 句柄索引（换板改这里） ---- */

const uint8_t Euuen_CAN_ConvertTable[EUUEN_CAN_NUM] = {
    0, /* EUUEN_CAN_1 -> BSP_CAN_1 */
    1, /* EUUEN_CAN_2 -> BSP_CAN_2 */
};

/* ---- 驱动状态：每个 EM_CAN_ID 对应一个 BSP_CAN 实例 ---- */

static BSP_CAN s_can[EUUEN_CAN_NUM];
static uint8_t s_can_inited[EUUEN_CAN_NUM];
static uint8_t s_rx_registered[EUUEN_CAN_NUM];
static EM_CAN_RxCallback s_rx_cb[EUUEN_CAN_NUM];

/* BSP 回调没有通道参数，因此每个通道使用独立的 trampoline */
static void EM_CAN_1_RxTrampoline(uint32_t can_id, uint8_t *data,
                                  uint8_t length) {
  if (s_rx_cb[EUUEN_CAN_1] != NULL) {
    s_rx_cb[EUUEN_CAN_1](EUUEN_CAN_1, can_id, data, length);
  }
}

static void EM_CAN_2_RxTrampoline(uint32_t can_id, uint8_t *data,
                                  uint8_t length) {
  if (s_rx_cb[EUUEN_CAN_2] != NULL) {
    s_rx_cb[EUUEN_CAN_2](EUUEN_CAN_2, can_id, data, length);
  }
}

static BSP_CAN *EM_CAN_Get(EM_CAN_ID can) {
  if (can < 0 || can >= EUUEN_CAN_NUM) {
    return NULL;
  }
  if (!s_can_inited[can]) {
    BSP_State state = BSP_ERROR;
    switch (can) {
    case EUUEN_CAN_1:
      state = BSP_CAN_1_Init(&s_can[can]);
      break;
    case EUUEN_CAN_2:
      state = BSP_CAN_2_Init(&s_can[can]);
      break;
    default:
      return NULL;
    }
    if (state != BSP_OK) {
      return NULL;
    }
    s_can_inited[can] = 1;
  }
  return &s_can[can];
}

/* 新 BSP 的 AddRxCallback 同时负责注册和使能通知，因此每个通道只注册一次 */
static void EM_CAN_RegisterRx(EM_CAN_ID can) {
  if (s_rx_registered[can]) {
    return;
  }
  BSP_State state = BSP_ERROR;
  switch (can) {
  case EUUEN_CAN_1:
    state = BSP_CAN_1_AddRxCallback(EM_CAN_1_RxTrampoline);
    break;
  case EUUEN_CAN_2:
    state = BSP_CAN_2_AddRxCallback(EM_CAN_2_RxTrampoline);
    break;
  default:
    return;
  }
  if (state == BSP_OK) {
    s_rx_registered[can] = 1;
  }
}

/* ---- 驱动 API ---- */

void EM_CAN_Init(EM_CAN_ID can) {
  if (can < 0 || can >= EUUEN_CAN_NUM) {
    return;
  }
  if (s_can_inited[can]) {
    LOG_WARN("[EM_CAN_Init] can %d already initialized, skipped", (int)can);
    return;
  }
  (void)EM_CAN_Get(can);
}

void EM_CAN_SetRxHandler(EM_CAN_ID can, EM_CAN_RxCallback callback) {
  if (can < 0 || can >= EUUEN_CAN_NUM || callback == NULL) {
    return;
  }
  s_rx_cb[can] = callback;
  if (EM_CAN_Get(can) == NULL) {
    return;
  }
  EM_CAN_RegisterRx(can);
}

void EM_CAN_EnableRxHandler(EM_CAN_ID can) {
  if (can < 0 || can >= EUUEN_CAN_NUM) {
    return;
  }
  if (EM_CAN_Get(can) == NULL) {
    return;
  }
  EM_CAN_RegisterRx(can);
}

void EM_CAN_SendStdFrame(EM_CAN_ID can, uint16_t can_id, const uint8_t *data,
                         size_t length, float timeout) {
  if (data == NULL || length == 0) {
    return;
  }
  BSP_CAN *bsp_can = EM_CAN_Get(can);
  if (bsp_can == NULL) {
    return;
  }
  BSP_CAN_SendStdFrame(bsp_can, can_id, (uint8_t *)data, length, timeout);
}
