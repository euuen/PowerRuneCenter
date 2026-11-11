#pragma once

#include <euuen_mod/interface/gpio.h>
#include <euuen_mod/interface/i2c.h>
#include <euuen_mod/peripheral/i2c_ex.h>
#include <stdint.h>

/* IST8310 三轴磁力计（I2C，7 位地址常见 0x0E）
 * 单次测量模式：每次 Read 都先写 CNTL1=0x01 触发，等 DRDY，再读 0x03~0x08。 */

typedef struct IST8310_t IST8310_t;

struct IST8310_t {
  EM_I2C_Ex_t i2c;      /* 绑定的 I2C 实例（含从机地址） */
  int16_t x, y, z;      /* 原始 LSB（2 的补码） */
  float mx, my, mz;     /* 磁场强度 uT */
};

/* 初始化并配置（释放复位 + PDCNTL/AVGCNTL）；失败返回 NULL
 * reset：硬复位脚（RSTN 低有效），无复位脚传 EM_GPIO_NONE */
IST8310_t *IST8310_Init(EM_I2C_ID i2c, uint8_t dev_addr, EM_GPIO_ID reset);

/* 阻塞读取一次三轴数据；成功返回 1、失败/超时返回 0
 * 出参单位：x/y/z 均为 uT（微特斯拉），可传 NULL 表示不需要该轴 */
uint8_t IST8310_Read(IST8310_t *self, float *x, float *y, float *z);
