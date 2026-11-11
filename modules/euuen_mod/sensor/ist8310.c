#include "ist8310.h"
#include <euuen_mod/debug/Logger.h>
#include <euuen_mod/euuen_mod.h>
#include <euuen_mod/interface/sys.h>
#include <euuen_mod/peripheral/gpio.h>
#include <stddef.h>
#include <string.h>

/* ============================ 寄存器/常量 ============================ */

#define IST8310_STAT1 0x02
#define IST8310_DATAXL 0x03 /* 0x03~0x08：X/Y/Z 低高字节 */
#define IST8310_CNTL1 0x0A
#define IST8310_CNTL2 0x0B
#define IST8310_AVGCNTL 0x41
#define IST8310_PDCNTL 0x42

#define IST8310_CNTL1_SINGLE 0x01 /* 单次测量 */
#define IST8310_STAT1_DRDY 0x01   /* 数据就绪 */

#define IST8310_PDCNTL_NORMAL 0xC0 /* 脉冲宽度 normal（性能优化） */
#define IST8310_AVGCNTL_AVG 0x24   /* 增大内部平均次数，降噪 */

#define IST8310_LSB_PER_UT 3.3f /* 灵敏度 3.3 LSB/uT */
#define IST8310_DRDY_TIMEOUT_MS 10

#define IST8310_RESET_LOW_MS 10
#define IST8310_RESET_RELEASE_MS 20

/* ============================ 读写辅助 ============================ */

static uint8_t ist8310_write(IST8310_t *self, uint8_t reg, uint8_t val) {
  return EM_I2C_Ex_MemWrite(&self->i2c, reg, EM_I2C_MEMADD_8BIT, &val, 1);
}

static uint8_t ist8310_read(IST8310_t *self, uint8_t reg, uint8_t *buf,
                            uint16_t len) {
  return EM_I2C_Ex_MemRead(&self->i2c, reg, EM_I2C_MEMADD_8BIT, buf, len);
}

/* ============================ 公共 API ============================ */

IST8310_t *IST8310_Init(EM_I2C_ID i2c, uint8_t dev_addr, EM_GPIO_ID reset) {
  IST8310_t *self =
      (IST8310_t *)MemAllocator_Alloc(&ram_allocator, sizeof(IST8310_t));
  if (self == NULL) {
    LOG_ERROR("[IST8310_Init] instance alloc failed");
    return NULL;
  }
  memset(self, 0, sizeof(IST8310_t));

  /* 释放硬复位（RSTN 低有效）：拉低 -> 等待 -> 拉高 -> 等待稳定 */
  if (reset != EM_GPIO_NONE) {
    EM_GPIO_WritePin(reset, EM_GPIO_RESET);
    EM_Delay_ms(IST8310_RESET_LOW_MS);
    EM_GPIO_WritePin(reset, EM_GPIO_SET);
    EM_Delay_ms(IST8310_RESET_RELEASE_MS);
  }

  EM_I2C_Ex_Init(&self->i2c, i2c, dev_addr);

  /* 初始配置：脉冲宽度 normal + 增大平均次数降噪 */
  if (!ist8310_write(self, IST8310_PDCNTL, IST8310_PDCNTL_NORMAL) ||
      !ist8310_write(self, IST8310_AVGCNTL, IST8310_AVGCNTL_AVG)) {
    LOG_ERROR("[IST8310_Init] config failed");
    MemAllocator_Free(&ram_allocator, self);
    return NULL;
  }

  return self;
}

/* 出参 x/y/z 单位：uT */
uint8_t IST8310_Read(IST8310_t *self, float *x, float *y, float *z) {
  if (self == NULL) {
    return 0;
  }

  /* 1. 触发一次单次测量 */
  if (!ist8310_write(self, IST8310_CNTL1, IST8310_CNTL1_SINGLE)) {
    LOG_ERROR("[IST8310] trigger failed");
    return 0;
  }

  /* 2. 阻塞等待 DRDY（超时保护） */
  uint32_t start = (uint32_t)EM_GetTimestamp_ms();
  uint8_t stat = 0;
  for (;;) {
    if (!ist8310_read(self, IST8310_STAT1, &stat, 1)) {
      LOG_ERROR("[IST8310] read STAT1 failed");
      return 0;
    }
    if (stat & IST8310_STAT1_DRDY) {
      break;
    }
    if ((uint32_t)EM_GetTimestamp_ms() - start > IST8310_DRDY_TIMEOUT_MS) {
      LOG_ERROR("[IST8310] DRDY timeout");
      return 0;
    }
    EM_Delay_ms(1);
  }

  /* 3. 连续读 6 字节（低字节在前） */
  uint8_t buf[6];
  if (!ist8310_read(self, IST8310_DATAXL, buf, 6)) {
    LOG_ERROR("[IST8310] read data failed");
    return 0;
  }

  self->x = (int16_t)((uint16_t)buf[1] << 8 | buf[0]);
  self->y = (int16_t)((uint16_t)buf[3] << 8 | buf[2]);
  self->z = (int16_t)((uint16_t)buf[5] << 8 | buf[4]);

  self->mx = (float)self->x / IST8310_LSB_PER_UT;
  self->my = (float)self->y / IST8310_LSB_PER_UT;
  self->mz = (float)self->z / IST8310_LSB_PER_UT;

  if (x != NULL) {
    *x = self->mx;
  }
  if (y != NULL) {
    *y = self->my;
  }
  if (z != NULL) {
    *z = self->mz;
  }
  return 1;
}
