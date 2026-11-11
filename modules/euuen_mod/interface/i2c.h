#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* 与 BSP 解耦的 I2C 枚举 */
typedef enum {
  EM_I2C_3 = 0,
  EM_I2C_NUM,
} EM_I2C_ID;

/* 寄存器地址宽度 */
typedef enum {
  EM_I2C_MEMADD_8BIT = 0,
  EM_I2C_MEMADD_16BIT,
} EM_I2C_MemAddrSize;

/* EM_I2C_ID -> BSP_I2C_ID 转换表 */
extern const uint8_t EM_I2C_ConvertTable[EM_I2C_NUM];

/* ---- 驱动 API：只做 BSP 包装 + 枚举映射（上层 peripheral 使用） ----
 * dev_addr 为 7 位从机地址 */

void EM_I2C_Init(EM_I2C_ID i2c);
uint8_t EM_I2C_Transmit(EM_I2C_ID i2c, uint8_t dev_addr, const uint8_t *data,
                        uint16_t length);
uint8_t EM_I2C_Receive(EM_I2C_ID i2c, uint8_t dev_addr, uint8_t *data,
                       uint16_t length);
uint8_t EM_I2C_MemWrite(EM_I2C_ID i2c, uint8_t dev_addr, uint16_t mem_addr,
                        EM_I2C_MemAddrSize mem_size, const uint8_t *data,
                        uint16_t length);
uint8_t EM_I2C_MemRead(EM_I2C_ID i2c, uint8_t dev_addr, uint16_t mem_addr,
                       EM_I2C_MemAddrSize mem_size, uint8_t *data,
                       uint16_t length);

#ifdef __cplusplus
}
#endif
