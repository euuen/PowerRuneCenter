#include "can.h"
#include "type.h"
#include "stm32f4xx_hal_can.h"
#include "stm32f4xx_hal_def.h"
#include "sys.h"
#include <can.h>
#include <stdint.h>
#include "parameter.h"

#define CAN_1_INITED_MASK (0x1 << 0)
#define CAN_2_INITED_MASK (0x1 << 1)


static uint8_t check_can_inited = 0;

static BSP_CAN_RxCallback bsp_can_1_rx_callbacks[BSP_CAN_1_MAX_RX_CALLBACKS] = {
    0};
static BSP_CAN_RxCallback bsp_can_2_rx_callbacks[BSP_CAN_2_MAX_RX_CALLBACKS] = {
    0};

BSP_State BSP_CAN_1_Init(BSP_CAN *bsp_can) {
  if (!(check_can_inited & CAN_1_INITED_MASK)) {
    MX_CAN1_Init();
    check_can_inited |= CAN_1_INITED_MASK;
  }
  bsp_can->hcan = &hcan1;

  CAN_FilterTypeDef sFilterConfig;
  sFilterConfig.FilterIdHigh = 0;
  sFilterConfig.FilterIdLow = 0;
  sFilterConfig.FilterMaskIdHigh = 0;
  sFilterConfig.FilterMaskIdLow = 0;
  sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  sFilterConfig.FilterBank = 0;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.SlaveStartFilterBank = 14;
  HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig);

  if (HAL_CAN_Start(&hcan1) == HAL_OK) {
    return BSP_OK;
  } else {
    return BSP_ERROR;
  }
}

BSP_State BSP_CAN_2_Init(BSP_CAN *bsp_can) {
  if (!(check_can_inited & CAN_2_INITED_MASK)) {
    MX_CAN2_Init();
    check_can_inited |= CAN_2_INITED_MASK;
  }
  bsp_can->hcan = &hcan2;

  CAN_FilterTypeDef sFilterConfig;
  sFilterConfig.FilterIdHigh = 0;
  sFilterConfig.FilterIdLow = 0;
  sFilterConfig.FilterMaskIdHigh = 0;
  sFilterConfig.FilterMaskIdLow = 0;
  sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO1;
  sFilterConfig.FilterBank = 14;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.SlaveStartFilterBank = 14;
  HAL_CAN_ConfigFilter(&hcan2, &sFilterConfig);

  if (HAL_CAN_Start(&hcan2) == HAL_OK) {
    return BSP_OK;
  } else {
    return BSP_ERROR;
  }
}

static BSP_State BSP_CAN_AddRxCallback(BSP_CAN_RxCallback *callbacks,
                                       uint8_t max_callbacks,
                                       BSP_CAN_RxCallback callback) {
  if (callback == NULL) {
    return BSP_ERROR;
  }
  for (uint8_t i = 0; i < max_callbacks; i++) {
    if (callbacks[i] == NULL) {
      callbacks[i] = callback;
      return BSP_OK;
    }
  }
  return BSP_ERROR;
}

BSP_State BSP_CAN_1_AddRxCallback(BSP_CAN_RxCallback callback) {
  BSP_State ret = BSP_CAN_AddRxCallback(bsp_can_1_rx_callbacks,
                                        BSP_CAN_1_MAX_RX_CALLBACKS, callback);
  if (ret == BSP_OK) {
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
  }
  return ret;
}

BSP_State BSP_CAN_2_AddRxCallback(BSP_CAN_RxCallback callback) {
  BSP_State ret = BSP_CAN_AddRxCallback(bsp_can_2_rx_callbacks,
                                        BSP_CAN_2_MAX_RX_CALLBACKS, callback);
  if (ret == BSP_OK) {
    HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO1_MSG_PENDING);
  }
  return ret;
}

BSP_State BSP_CAN_SendStdFrame(BSP_CAN *bsp_can, uint16_t can_id, uint8_t *data,
                               size_t length, float timeout) {
  CAN_TxHeaderTypeDef canTxHeader = {0};
  uint32_t txMailbox = 0;

  canTxHeader.StdId = can_id;
  canTxHeader.IDE = CAN_ID_STD;
  canTxHeader.RTR = CAN_RTR_DATA;
  canTxHeader.DLC = length;

  float start_time = BSP_GetTimestamp_ms();
  while (HAL_CAN_GetTxMailboxesFreeLevel(bsp_can->hcan) != 3) {
    if (BSP_GetTimestamp_ms() - start_time > timeout)
      return BSP_TIMEOUT;
  }

  HAL_CAN_AddTxMessage(bsp_can->hcan, &canTxHeader, data, &txMailbox);
  return BSP_OK;
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
  if (hcan != &hcan1) {
    return;
  }

  CAN_RxHeaderTypeDef rxHeader;
  uint8_t rxData[8];
  HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxData);

  uint32_t id = (rxHeader.IDE == CAN_ID_EXT) ? rxHeader.ExtId : rxHeader.StdId;
  for (uint8_t i = 0; i < BSP_CAN_1_MAX_RX_CALLBACKS; i++) {
    if (bsp_can_1_rx_callbacks[i] != NULL) {
      bsp_can_1_rx_callbacks[i](id, rxData, rxHeader.DLC);
    }
  }
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan) {
  if (hcan != &hcan2) {
    return;
  }

  CAN_RxHeaderTypeDef rxHeader;
  uint8_t rxData[8];
  HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &rxHeader, rxData);

  uint32_t id = (rxHeader.IDE == CAN_ID_EXT) ? rxHeader.ExtId : rxHeader.StdId;
  for (uint8_t i = 0; i < BSP_CAN_2_MAX_RX_CALLBACKS; i++) {
    if (bsp_can_2_rx_callbacks[i] != NULL) {
      bsp_can_2_rx_callbacks[i](id, rxData, rxHeader.DLC);
    }
  }
}
