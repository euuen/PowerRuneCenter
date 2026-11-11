/*
Mahony 测试：BMI088 六轴融合
JustFloat 4 通道：w, x, y, z
*/

#include <c_board_bsp/usart.h>
#include <cmsis_os.h>
#include <euuen_mod/debug/Logger.h>
#include <euuen_mod/euuen_mod.h>
#include <stdint.h>

/* 本板：SPI1，Accel CS=PA4(EM_GPIO_1)，Gyro CS=PB0(EM_GPIO_2) */
static BMI088_t *bmi088;
static Mahony_t *mahony;
static BSP_USART bsp_usart_1;
static Logger logger;
static uint64_t last_us;

void APP_Setup() {
  EM_MakeContext();

  BSP_USART_1_Init(&bsp_usart_1);
  Logger_Init(&logger, &bsp_usart_1);

  bmi088 = BMI088_Init(EM_SPI_1, EM_GPIO_1, EM_GPIO_2, BMI088_ACC_RANGE_6G,
                       BMI088_GYRO_RANGE_2000DPS);
  mahony = Mahony_Init(1.0f, 0.0f);
  if (bmi088 == NULL || mahony == NULL) {
    Logger_Log(&logger, "mahony test init failed\n");
    return;
  }

  last_us = EM_GetTimestamp_us();
  Logger_Log(&logger, "mahony test inited\n");
}

void APP_Loop() {
  if (bmi088 == NULL || mahony == NULL) {
    osDelay(10);
    return;
  }

  uint64_t now_us = EM_GetTimestamp_us();
  float dt_s = (float)(now_us - last_us) * 1e-6f;
  last_us = now_us;
  if (dt_s <= 0.0f || dt_s > 0.1f) {
    dt_s = 0.01f; /* 异常间隔时退回标称周期 */
  }

  float gyro[3];  /* °/s */
  float accel[3]; /* m/s^2 */
  BMI088_GetGyro(bmi088, &gyro[0], &gyro[1], &gyro[2]);
  BMI088_GetAccel(bmi088, &accel[0], &accel[1], &accel[2]);

  Mahony_Update(mahony, gyro, accel, dt_s);

  float q[4]; /* w, x, y, z */
  Mahony_GetQuaternion(mahony, &q[0], &q[1], &q[2], &q[3]);
  Logger_Log_JustFloat(&logger, q, 4);

  osDelay(10);
}
