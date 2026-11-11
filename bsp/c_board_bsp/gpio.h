#pragma once

#include "type.h"
#include <main.h>
#include <stdint.h>

/* 板载 GPIO 编号（只列业务会用到的引脚） */
typedef enum {
  BSP_GPIO_CS1_ACCEL = 0, /* CS1_ACCEL: PA4 */
  BSP_GPIO_CS1_GYRO,      /* CS1_GYRO:  PB0 */
  BSP_GPIO_INT_ACC,       /* INT_ACC:   PC4 */
  BSP_GPIO_INT_GYRO,      /* INT_GYRO:  PC5 */
  BSP_GPIO_MAG_RST,       /* MAG_RST:   PG6 */
  BSP_GPIO_KEY,           /* KEY:       PA0 */
  BSP_GPIO_NUM,
} BSP_GPIO_ID;

/* GPIO 实例：调用方持有，Init 时按编号绑定到具体端口/引脚 */
typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} BSP_GPIO;

BSP_State BSP_GPIO_Init(BSP_GPIO *bsp_gpio, BSP_GPIO_ID id);
void BSP_GPIO_WritePin(BSP_GPIO *bsp_gpio, GPIO_PinState state);
void BSP_GPIO_TogglePin(BSP_GPIO *bsp_gpio);
GPIO_PinState BSP_GPIO_ReadPin(BSP_GPIO *bsp_gpio);
