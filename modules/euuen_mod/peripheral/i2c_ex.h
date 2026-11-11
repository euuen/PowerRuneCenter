#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <euuen_mod/interface/i2c.h>
#include <stdint.h>

/* I2C 实例：绑定一个硬件 I2C + 从机地址（7 位）。
 * 可内嵌在使用方结构体里；读写时内部自动锁住硬件 I2C（总线被多设备共用）。 */
typedef struct EM_I2C_Ex_t {
  EM_I2C_ID i2c;    /* 挂在哪个硬件 I2C */
  uint8_t dev_addr; /* 7 位从机地址 */
} EM_I2C_Ex_t;

void EM_I2C_Ex_Init(EM_I2C_Ex_t *self, EM_I2C_ID i2c, uint8_t dev_addr);

uint8_t EM_I2C_Ex_Transmit(EM_I2C_Ex_t *self, const uint8_t *data,
                           uint16_t length);
uint8_t EM_I2C_Ex_Receive(EM_I2C_Ex_t *self, uint8_t *data, uint16_t length);
uint8_t EM_I2C_Ex_MemWrite(EM_I2C_Ex_t *self, uint16_t mem_addr,
                           EM_I2C_MemAddrSize mem_size, const uint8_t *data,
                           uint16_t length);
uint8_t EM_I2C_Ex_MemRead(EM_I2C_Ex_t *self, uint16_t mem_addr,
                          EM_I2C_MemAddrSize mem_size, uint8_t *data,
                          uint16_t length);

#ifdef __cplusplus
}
#endif
