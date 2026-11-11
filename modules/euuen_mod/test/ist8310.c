/*
IST8310 测试：仅输出磁力计测量值
JustFloat 3 通道：mag_x, mag_y, mag_z (uT)
*/

#include <c_board_bsp/sys.h>
#include <c_board_bsp/usart.h>
#include <cmsis_os.h>
#include <euuen_mod/debug/Logger.h>
#include <euuen_mod/euuen_mod.h>
#include <stdint.h>

/* 本板：I2C3，7 位地址 0x0E；硬复位脚 MAG_RST=PG6=EM_GPIO_5 */
static IST8310_t *mag;
static BSP_USART bsp_usart_1;
static Logger logger;

void APP_Setup() {
  EM_MakeContext();

  BSP_USART_1_Init(&bsp_usart_1);
  Logger_Init(&logger, &bsp_usart_1);

  mag = IST8310_Init(EM_I2C_3, 0x0E, EM_GPIO_5);
  if (mag == NULL) {
    Logger_Log(&logger, "ist8310 init failed\n");
    return;
  }
  Logger_Log(&logger, "ist8310 inited\n");
}

void APP_Loop() {
  if (mag == NULL) {
    osDelay(1);
    return;
  }

  float x, y, z; /* uT */
  if (IST8310_Read(mag, &x, &y, &z)) {
    float out[3] = {x, y, z};
    Logger_Log_JustFloat(&logger, out, 3);
  }

  osDelay(10);
}
