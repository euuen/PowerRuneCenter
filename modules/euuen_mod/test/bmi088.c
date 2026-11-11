/*
BMI088 测试：仅输出 IMU 自身测量值
JustFloat 6 通道：
  gyro_x, gyro_y, gyro_z (°/s)
  acc_x,  acc_y,  acc_z  (m/s^2)
*/

#include <c_board_bsp/sys.h>
#include <c_board_bsp/usart.h>
#include <cmsis_os.h>
#include <euuen_mod/debug/Logger.h>
#include <euuen_mod/euuen_mod.h>
#include <stdint.h>

/* 本板：SPI1，Accel CS=PA4(EM_GPIO_1)，Gyro CS=PB0(EM_GPIO_2) */
static BMI088_t *bmi088;
static BSP_USART bsp_usart_1;
static Logger logger;

void APP_Setup() {
  EM_MakeContext();

  BSP_USART_1_Init(&bsp_usart_1);
  Logger_Init(&logger, &bsp_usart_1);

  bmi088 = BMI088_Init(EM_SPI_1, EM_GPIO_1, EM_GPIO_2, BMI088_ACC_RANGE_6G,
                       BMI088_GYRO_RANGE_2000DPS);
  if (bmi088 == NULL) {
    Logger_Log(&logger, "bmi088 init failed\n");
    return;
  }
  Logger_Log(&logger, "bmi088 inited\n");
}

void APP_Loop() {
  if (bmi088 == NULL) {
    osDelay(1);
    return;
  }

  float gyro[3]; /* °/s */
  float acc[3];  /* m/s^2 */
  BMI088_GetGyro(bmi088, &gyro[0], &gyro[1], &gyro[2]);
  BMI088_GetAccel(bmi088, &acc[0], &acc[1], &acc[2]);

  float out[6] = {gyro[0], gyro[1], gyro[2], acc[0], acc[1], acc[2]};
  Logger_Log_JustFloat(&logger, out, 6);

  osDelay(1);
}
