#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <euuen_mod/interface/can.h>
#include <stddef.h>
#include <stdint.h>

/* 业务层回调（不依赖 BSP / 驱动） */
typedef void (*EM_CAN_Ex_RxHandler)(uint32_t can_id, uint8_t *data,
                                    uint8_t length);

/* ---- 扩展能力公共 API（业务模块调用） ---- */

void EM_CAN_Ex_Init(EM_CAN_ID can);
void EM_CAN_Ex_EnableRxHandler(EM_CAN_ID can);
void EM_CAN_Ex_AddRxHandler(EM_CAN_ID can, EM_CAN_Ex_RxHandler handler);
void EM_CAN_Ex_SendStdFrame(EM_CAN_ID can, uint16_t can_id, const uint8_t *data,
                            size_t length, float timeout);

#ifdef __cplusplus
}
#endif
