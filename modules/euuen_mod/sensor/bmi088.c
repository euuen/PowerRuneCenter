#include "bmi088.h"
#include <euuen_mod/debug/Logger.h>
#include <euuen_mod/euuen_mod.h>
#include <euuen_mod/interface/sys.h>
#include <euuen_mod/peripheral/spi_ex.h>
#include <stddef.h>
#include <string.h>

/* ============================ 器件常量 ============================ */

/* 器件 ID */
#define BMI088_ACC_CHIP_ID_VAL 0x1E
#define BMI088_GYRO_CHIP_ID_VAL 0x0F

/* 寄存器地址 */
#define BMI088_ACC_CHIP_ID 0x00
#define BMI088_ACC_X_LSB 0x12
#define BMI088_TEMP_MSB 0x22
#define BMI088_TEMP_LSB 0x23
#define BMI088_ACC_CONF 0x40
#define BMI088_ACC_RANGE 0x41
#define BMI088_ACC_PWR_CONF 0x7C
#define BMI088_ACC_PWR_CTRL 0x7D
#define BMI088_ACC_SOFTRESET 0x7E

#define BMI088_GYRO_CHIP_ID 0x00
#define BMI088_GYRO_RATE_X_LSB 0x02
#define BMI088_GYRO_RANGE 0x0F
#define BMI088_GYRO_BANDWIDTH 0x10
#define BMI088_GYRO_SOFTRESET 0x14

/* 配置值 */
#define BMI088_ACC_CONF_200HZ_NORMAL 0xA9 /* bwp=0xA(normal), odr=0x9(200Hz) */
#define BMI088_GYRO_BW_1000HZ_116HZ 0x02

/* 软复位命令（手册 4.8 用 0xB6；部分版本 5.5.8 陀螺仪写 0x86） */
#define BMI088_ACC_SOFTRESET_CMD 0xB6
#define BMI088_GYRO_SOFTRESET_CMD 0xB6

/* 加速度计电源控制 */
#define BMI088_ACC_PWR_CTRL_ENABLE 0x04
#define BMI088_ACC_PWR_CONF_ACTIVE 0x00

/* 温度换算（11 位：uint11 = (MSB<<3)|(LSB>>5)，int11 为 11 位有符号） */
#define BMI088_TEMP_UINT11_MAX 1023
#define BMI088_TEMP_UINT11_SPAN 2048
#define BMI088_TEMP_LSB_PER_DEG 0.125f
#define BMI088_TEMP_OFFSET_DEG 23.0f

/* 延时 */
#define BMI088_SOFTRESET_DELAY_MS 1 /* 使能后的等待 */
#define BMI088_ACC_SOFTRESET_DELAY_MS 50
#define BMI088_GYRO_SOFTRESET_DELAY_MS 30
#define BMI088_WRITE_GAP_US 2 /* normal 模式两次写之间 >= 2us */

#define BMI088_G_TO_MPS2 9.80665f

/* 单次传输最大长度：accel burst = 6 + 2 头 = 8 */
#define BMI088_XFER_BUF 8

/* 由量程决定的标度（见数据手册） */
static const float s_acc_lsb_per_g[4] = {10920.0f, 5460.0f, 2730.0f, 1365.0f};
static const float s_gyro_lsb_per_dps[5] = {16.384f, 32.768f, 65.536f, 131.072f,
                                            262.144f};

/* ============================ 底层 SPI 事务 ============================
 * 收发缓冲由本层局部数组提供，按次传给 EM_SPI_Ex_SendReceive；
 * 片选/总线锁由 SPI 实例内部处理。每个事务返回 1 成功 / 0 失败 */

/* 加速度计单寄存器写 */
static uint8_t bmi088_acc_write(BMI088_t *self, uint8_t reg, uint8_t data) {
  uint8_t tx[2] = {(uint8_t)(reg & 0x7F), data};
  uint8_t rx[2];
  uint8_t ok = EM_SPI_Ex_SendReceive(&self->accel, tx, rx, 2);
  EM_Delay_us(BMI088_WRITE_GAP_US);
  if (!ok) {
    LOG_ERROR("[BMI088] acc write reg 0x%02X failed", reg);
  }
  return ok;
}

/* 陀螺仪单寄存器写 */
static uint8_t bmi088_gyro_write(BMI088_t *self, uint8_t reg, uint8_t data) {
  uint8_t tx[2] = {(uint8_t)(reg & 0x7F), data};
  uint8_t rx[2];
  uint8_t ok = EM_SPI_Ex_SendReceive(&self->gyro, tx, rx, 2);
  EM_Delay_us(BMI088_WRITE_GAP_US);
  if (!ok) {
    LOG_ERROR("[BMI088] gyro write reg 0x%02X failed", reg);
  }
  return ok;
}

/* 加速度计单寄存器读：命令 + dummy + 数据 */
static uint8_t bmi088_acc_read(BMI088_t *self, uint8_t reg, uint8_t *out) {
  uint8_t tx[3] = {(uint8_t)(reg | 0x80), 0x00, 0x00};
  uint8_t rx[3];
  uint8_t ok = EM_SPI_Ex_SendReceive(&self->accel, tx, rx, 3);
  if (ok && out != NULL) {
    *out = rx[2]; /* 第 3 字节才是数据 */
  }
  EM_Delay_us(BMI088_WRITE_GAP_US);
  if (!ok) {
    LOG_ERROR("[BMI088] acc read reg 0x%02X failed", reg);
  }
  return ok;
}

/* 陀螺仪单寄存器读：命令 + 数据 */
static uint8_t bmi088_gyro_read(BMI088_t *self, uint8_t reg, uint8_t *out) {
  uint8_t tx[2] = {(uint8_t)(reg | 0x80), 0x00};
  uint8_t rx[2];
  uint8_t ok = EM_SPI_Ex_SendReceive(&self->gyro, tx, rx, 2);
  if (ok && out != NULL) {
    *out = rx[1]; /* 第 2 字节即数据 */
  }
  EM_Delay_us(BMI088_WRITE_GAP_US);
  if (!ok) {
    LOG_ERROR("[BMI088] gyro read reg 0x%02X failed", reg);
  }
  return ok;
}

/* 加速度计 burst 读：命令 + 2 字节头，之后才是数据 */
static uint8_t bmi088_acc_read_multi(BMI088_t *self, uint8_t reg, uint8_t *buf,
                                     uint16_t len) {
  if (len + 2U > BMI088_XFER_BUF) {
    return 0;
  }
  uint8_t tx[BMI088_XFER_BUF] = {0};
  uint8_t rx[BMI088_XFER_BUF] = {0};
  tx[0] = (uint8_t)(reg | 0x80);

  uint8_t ok = EM_SPI_Ex_SendReceive(&self->accel, tx, rx, (uint16_t)(len + 2U));
  if (ok) {
    memcpy(buf, &rx[2], len);
  }
  EM_Delay_us(BMI088_WRITE_GAP_US);
  if (!ok) {
    LOG_ERROR("[BMI088] acc burst read reg 0x%02X len %u failed", reg,
              (unsigned)len);
  }
  return ok;
}

/* 陀螺仪 burst 读：命令后直接是数据 */
static uint8_t bmi088_gyro_read_multi(BMI088_t *self, uint8_t reg, uint8_t *buf,
                                      uint16_t len) {
  if (len + 1U > BMI088_XFER_BUF) {
    return 0;
  }
  uint8_t tx[BMI088_XFER_BUF] = {0};
  uint8_t rx[BMI088_XFER_BUF] = {0};
  tx[0] = (uint8_t)(reg | 0x80);

  uint8_t ok = EM_SPI_Ex_SendReceive(&self->gyro, tx, rx, (uint16_t)(len + 1U));
  if (ok) {
    memcpy(buf, &rx[1], len);
  }
  EM_Delay_us(BMI088_WRITE_GAP_US);
  if (!ok) {
    LOG_ERROR("[BMI088] gyro burst read reg 0x%02X len %u failed", reg,
              (unsigned)len);
  }
  return ok;
}

/* ============================ 公共 API ============================ */

BMI088_t *BMI088_Init(EM_SPI_ID spi, EM_GPIO_ID cs_accel, EM_GPIO_ID cs_gyro,
                      BMI088_AccRange acc_range, BMI088_GyroRange gyro_range) {
  if (acc_range > BMI088_ACC_RANGE_24G ||
      gyro_range > BMI088_GYRO_RANGE_125DPS) {
    LOG_ERROR("[BMI088_Init] invalid range acc=%d gyro=%d", (int)acc_range,
              (int)gyro_range);
    return NULL;
  }

  BMI088_t *self =
      (BMI088_t *)MemAllocator_Alloc(&ram_allocator, sizeof(BMI088_t));
  if (self == NULL) {
    LOG_ERROR("[BMI088_Init] instance alloc failed");
    return NULL;
  }
  memset(self, 0, sizeof(BMI088_t));
  self->acc_range = acc_range;
  self->gyro_range = gyro_range;
  self->acc_lsb_per_g = s_acc_lsb_per_g[acc_range];
  self->gyro_lsb_per_dps = s_gyro_lsb_per_dps[gyro_range];

  /* 建立两个 SPI 实例（同一硬件 SPI、不同片选），各实例自带总线锁与片选 */
  EM_SPI_Ex_Init(&self->accel, spi, cs_accel);
  EM_SPI_Ex_Init(&self->gyro, spi, cs_gyro);

  /* 1. 一次读产生 CS 上升沿，把加速度计从 I2C 切到 SPI（本次数据无效） */
  (void)bmi088_acc_read(self, BMI088_ACC_CHIP_ID, NULL);

  /* 2. 校验器件 ID（读取失败 / ID 不符都报错） */
  uint8_t acc_id = 0;
  uint8_t gyro_id = 0;
  if (!bmi088_acc_read(self, BMI088_ACC_CHIP_ID, &acc_id) ||
      !bmi088_gyro_read(self, BMI088_GYRO_CHIP_ID, &gyro_id)) {
    LOG_ERROR("[BMI088_Init] chip id read failed");
    MemAllocator_Free(&ram_allocator, self);
    return NULL;
  }
  if (acc_id != BMI088_ACC_CHIP_ID_VAL ||
      gyro_id != BMI088_GYRO_CHIP_ID_VAL) {
    LOG_ERROR("[BMI088_Init] chip id mismatch acc=0x%02X gyro=0x%02X", acc_id,
              gyro_id);
    MemAllocator_Free(&ram_allocator, self);
    return NULL;
  }

  /* 3. 加速度计：软复位 -> 重新切 SPI -> 使能 -> 退出 suspend -> 量程/ODR
   *    注意：加速度计软复位后会退回 I2C 接口，必须再发一次 CS 上升沿切回 SPI，
   *    否则后续所有 SPI 配置写都会被忽略，数据恒为 0。 */
  bmi088_acc_write(self, BMI088_ACC_SOFTRESET, BMI088_ACC_SOFTRESET_CMD);
  EM_Delay_ms(BMI088_ACC_SOFTRESET_DELAY_MS);
  (void)bmi088_acc_read(self, BMI088_ACC_CHIP_ID, NULL); /* 重新切 SPI */
  bmi088_acc_write(self, BMI088_ACC_PWR_CTRL, BMI088_ACC_PWR_CTRL_ENABLE);
  EM_Delay_ms(BMI088_SOFTRESET_DELAY_MS);
  bmi088_acc_write(self, BMI088_ACC_PWR_CONF, BMI088_ACC_PWR_CONF_ACTIVE);
  EM_Delay_ms(BMI088_SOFTRESET_DELAY_MS);
  bmi088_acc_write(self, BMI088_ACC_RANGE, (uint8_t)acc_range);
  bmi088_acc_write(self, BMI088_ACC_CONF, BMI088_ACC_CONF_200HZ_NORMAL);

  /* 4. 陀螺仪：软复位 -> 量程/带宽
   *    注意：手册 4.8 与 5.5.8 对软复位值有 0xB6 / 0x86 两种说法，此处用 0xB6 */
  bmi088_gyro_write(self, BMI088_GYRO_SOFTRESET, BMI088_GYRO_SOFTRESET_CMD);
  EM_Delay_ms(BMI088_GYRO_SOFTRESET_DELAY_MS);
  bmi088_gyro_write(self, BMI088_GYRO_RANGE, (uint8_t)gyro_range);
  bmi088_gyro_write(self, BMI088_GYRO_BANDWIDTH, BMI088_GYRO_BW_1000HZ_116HZ);

  return self;
}

static uint8_t bmi088_get_accel_raw(BMI088_t *self, int16_t *x, int16_t *y,
                                    int16_t *z) {
  if (self == NULL || x == NULL || y == NULL || z == NULL) {
    return 0;
  }
  uint8_t buf[6];
  if (!bmi088_acc_read_multi(self, BMI088_ACC_X_LSB, buf, 6)) {
    LOG_ERROR("[BMI088] accel read failed");
    return 0;
  }

  /* 短临界区：保证写入共享字段/输出时不会被任务切换打断而产生半帧 */
  EM_EnterCritical();
  self->acc_x = (int16_t)((uint16_t)buf[1] << 8 | buf[0]);
  self->acc_y = (int16_t)((uint16_t)buf[3] << 8 | buf[2]);
  self->acc_z = (int16_t)((uint16_t)buf[5] << 8 | buf[4]);
  *x = self->acc_x;
  *y = self->acc_y;
  *z = self->acc_z;
  EM_ExitCritical();
  return 1;
}

static uint8_t bmi088_get_gyro_raw(BMI088_t *self, int16_t *x, int16_t *y,
                                   int16_t *z) {
  if (self == NULL || x == NULL || y == NULL || z == NULL) {
    return 0;
  }
  uint8_t buf[6];
  if (!bmi088_gyro_read_multi(self, BMI088_GYRO_RATE_X_LSB, buf, 6)) {
    LOG_ERROR("[BMI088] gyro read failed");
    return 0;
  }

  /* 短临界区：保证写入共享字段/输出时不会被任务切换打断而产生半帧 */
  EM_EnterCritical();
  self->gyro_x = (int16_t)((uint16_t)buf[1] << 8 | buf[0]);
  self->gyro_y = (int16_t)((uint16_t)buf[3] << 8 | buf[2]);
  self->gyro_z = (int16_t)((uint16_t)buf[5] << 8 | buf[4]);
  *x = self->gyro_x;
  *y = self->gyro_y;
  *z = self->gyro_z;
  EM_ExitCritical();
  return 1;
}

float BMI088_GetTemperature(BMI088_t *self) {
  if (self == NULL) {
    return 0.0f;
  }
  /* 温度寄存器 11 位，burst 读 2 字节避免 MSB/LSB 不同步 */
  uint8_t tbuf[2];
  if (!bmi088_acc_read_multi(self, BMI088_TEMP_MSB, tbuf, 2)) {
    LOG_ERROR("[BMI088] temperature read failed");
    return self->temperature; /* 返回上次有效值 */
  }
  uint16_t temp_uint11 = ((uint16_t)tbuf[0] << 3) | (tbuf[1] >> 5);
  int16_t temp_int11 = (temp_uint11 > BMI088_TEMP_UINT11_MAX)
                           ? (int16_t)(temp_uint11 - BMI088_TEMP_UINT11_SPAN)
                           : (int16_t)temp_uint11;
  float t = (float)temp_int11 * BMI088_TEMP_LSB_PER_DEG +
            BMI088_TEMP_OFFSET_DEG;
  EM_EnterCritical();
  self->temperature = t;
  EM_ExitCritical();
  return self->temperature;
}

void BMI088_GetAccel(BMI088_t *self, float *x, float *y, float *z) {
  if (self == NULL || x == NULL || y == NULL || z == NULL) {
    return;
  }
  int16_t rx, ry, rz;
  if (!bmi088_get_accel_raw(self, &rx, &ry, &rz)) {
    return; /* 读取失败已在上层打印 LOG_ERROR */
  }
  *x = (float)rx / self->acc_lsb_per_g * BMI088_G_TO_MPS2;
  *y = (float)ry / self->acc_lsb_per_g * BMI088_G_TO_MPS2;
  *z = (float)rz / self->acc_lsb_per_g * BMI088_G_TO_MPS2;
}

void BMI088_GetGyro(BMI088_t *self, float *x, float *y, float *z) {
  if (self == NULL || x == NULL || y == NULL || z == NULL) {
    return;
  }
  int16_t rx, ry, rz;
  if (!bmi088_get_gyro_raw(self, &rx, &ry, &rz)) {
    return; /* 读取失败已在上层打印 LOG_ERROR */
  }
  *x = (float)rx / self->gyro_lsb_per_dps;
  *y = (float)ry / self->gyro_lsb_per_dps;
  *z = (float)rz / self->gyro_lsb_per_dps;
}
