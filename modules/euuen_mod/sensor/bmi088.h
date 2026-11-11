#pragma once

#include <euuen_mod/interface/gpio.h>
#include <euuen_mod/interface/spi.h>
#include <euuen_mod/peripheral/spi_ex.h>
#include <stdint.h>

/* BMI088 六轴 IMU
 * 加速度计与陀螺仪是两套独立寄存器空间，分别用各自 CS 选择。
 * 内部持有两个 SPI 实例（accel / gyro），各自带片选，初始化时传入。 */

typedef enum {
  BMI088_ACC_RANGE_3G = 0x00,
  BMI088_ACC_RANGE_6G = 0x01,
  BMI088_ACC_RANGE_12G = 0x02,
  BMI088_ACC_RANGE_24G = 0x03,
} BMI088_AccRange;

typedef enum {
  BMI088_GYRO_RANGE_2000DPS = 0x00,
  BMI088_GYRO_RANGE_1000DPS = 0x01,
  BMI088_GYRO_RANGE_500DPS = 0x02,
  BMI088_GYRO_RANGE_250DPS = 0x03,
  BMI088_GYRO_RANGE_125DPS = 0x04,
} BMI088_GyroRange;

typedef struct BMI088_t BMI088_t;

struct BMI088_t {
  EM_SPI_Ex_t accel;            /* 加速度计 SPI 实例（含片选） */
  EM_SPI_Ex_t gyro;             /* 陀螺仪 SPI 实例（含片选） */
  BMI088_AccRange acc_range;    /* 当前加速度量程 */
  BMI088_GyroRange gyro_range;  /* 当前陀螺仪量程 */
  float acc_lsb_per_g;          /* 由量程决定的 LSB/g */
  float gyro_lsb_per_dps;       /* 由量程决定的 LSB/(°/s) */
  int16_t acc_x, acc_y, acc_z;  /* 最近一次加速度原始值 */
  int16_t gyro_x, gyro_y, gyro_z; /* 最近一次陀螺仪原始值 */
  float temperature;            /* 温度 ℃ */
};

/* 初始化（负责拉起 SPI、切换器件到 SPI、校验 ID、配置量程/带宽）；失败返回 NULL */
BMI088_t *BMI088_Init(EM_SPI_ID spi, EM_GPIO_ID cs_accel, EM_GPIO_ID cs_gyro,
                      BMI088_AccRange acc_range, BMI088_GyroRange gyro_range);

/* 温度读取（℃） */
float BMI088_GetTemperature(BMI088_t *self);

/* 物理量读取：加速度 m/s^2，角速度 °/s */
void BMI088_GetAccel(BMI088_t *self, float *x, float *y, float *z);
void BMI088_GetGyro(BMI088_t *self, float *x, float *y, float *z);
