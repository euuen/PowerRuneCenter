#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// 与 BSP 解耦的 GPIO 编号（换板只改 interface 实现的转换表）
typedef enum {
  EM_GPIO_NONE = 0, /* 不控制片选 */
  EM_GPIO_1,        /* -> BSP_GPIO_PA4  CS1_ACCEL */
  EM_GPIO_2,        /* -> BSP_GPIO_PB0  CS1_GYRO  */
  EM_GPIO_3,        /* -> BSP_GPIO_PC4  INT_ACC   */
  EM_GPIO_4,        /* -> BSP_GPIO_PC5  INT_GYRO  */
  EM_GPIO_5,        /* -> BSP_GPIO_PG6  MAG_RST   */
  EM_GPIO_6,        /* -> BSP_GPIO_PA0  KEY       */
  EM_GPIO_NUM,
} EM_GPIO_ID;

// 电平（不暴露 HAL 的 GPIO_PinState）
typedef enum {
  EM_GPIO_RESET = 0,
  EM_GPIO_SET,
} EM_GPIO_State;

// EM_GPIO_ID -> BSP_GPIO_ID 转换表
extern const uint8_t EM_GPIO_ConvertTable[EM_GPIO_NUM];

/* ---- 驱动 API：只做 BSP 包装 + 枚举映射（上层 peripheral 使用） ---- */

void EM_GPIO_WritePin(EM_GPIO_ID id, EM_GPIO_State state);
void EM_GPIO_TogglePin(EM_GPIO_ID id);
EM_GPIO_State EM_GPIO_ReadPin(EM_GPIO_ID id);

#ifdef __cplusplus
}
#endif
