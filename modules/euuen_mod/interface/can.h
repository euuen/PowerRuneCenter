#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

// 与 BSP 解耦的外设枚举
typedef enum {
  EUUEN_CAN_1 = 0,
  EUUEN_CAN_2,
  EUUEN_CAN_NUM,
} EM_CAN_ID;

// EUUEN 枚举 -> BSP 枚举转换表（值仅作句柄索引，具体含义见 interface 实现）
extern const uint8_t Euuen_CAN_ConvertTable[EUUEN_CAN_NUM];

/* 驱动回调：参数用 EUUEN 枚举，由 interface 负责 BSP id <-> EUUEN 枚举的映射 */
typedef void (*EM_CAN_RxCallback)(EM_CAN_ID can, uint32_t can_id, uint8_t *data,
                                  uint8_t length);

/* ---- 驱动 API：只做 BSP 包装 + 防重复初始化（上层 peripheral 使用） ---- */

void EM_CAN_Init(EM_CAN_ID can);
void EM_CAN_SetRxHandler(EM_CAN_ID can, EM_CAN_RxCallback callback);
void EM_CAN_EnableRxHandler(EM_CAN_ID can);
void EM_CAN_SendStdFrame(EM_CAN_ID can, uint16_t can_id, const uint8_t *data,
                         size_t length, float timeout);

#ifdef __cplusplus
}
#endif
