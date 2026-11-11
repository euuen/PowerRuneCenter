#pragma once

#include "type.h"
#include <main.h>
#include <stdint.h>

#define BSP_I2C_TIMEOUT_MS 10

/* I2C 实例：调用方持有，Init 时绑定到具体硬件 I2C */
typedef struct {
    I2C_HandleTypeDef *hi2c;
} BSP_I2C;

BSP_State BSP_I2C_3_Init(BSP_I2C *bsp_i2c);

/* dev_addr 为 7 位从机地址，内部自动左移 1 位成 HAL 需要的 8 位地址 */
BSP_State BSP_I2C_Transmit(BSP_I2C *bsp_i2c, uint8_t dev_addr,
                           const uint8_t *data, uint16_t length);
BSP_State BSP_I2C_Receive(BSP_I2C *bsp_i2c, uint8_t dev_addr, uint8_t *data,
                          uint16_t length);

/* mem_addr_size 取 HAL 的 I2C_MEMADD_SIZE_8BIT / I2C_MEMADD_SIZE_16BIT */
BSP_State BSP_I2C_MemWrite(BSP_I2C *bsp_i2c, uint8_t dev_addr,
                           uint16_t mem_addr, uint16_t mem_addr_size,
                           const uint8_t *data, uint16_t length);
BSP_State BSP_I2C_MemRead(BSP_I2C *bsp_i2c, uint8_t dev_addr, uint16_t mem_addr,
                          uint16_t mem_addr_size, uint8_t *data,
                          uint16_t length);
