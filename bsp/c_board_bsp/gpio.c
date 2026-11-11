#include "gpio.h"
#include "type.h"
#include <main.h>
#include <stdint.h>

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} BSP_GPIO_Map_t;

static const BSP_GPIO_Map_t bsp_gpio_map[BSP_GPIO_NUM] = {
    {CS1_ACCEL_GPIO_Port, CS1_ACCEL_Pin}, /* BSP_GPIO_CS1_ACCEL */
    {CS1_GYRO_GPIO_Port, CS1_GYRO_Pin},   /* BSP_GPIO_CS1_GYRO */
    {INT_ACC_GPIO_Port, INT_ACC_Pin},     /* BSP_GPIO_INT_ACC */
    {INT_GYRO_GPIO_Port, INT_GYRO_Pin},   /* BSP_GPIO_INT_GYRO */
    {MAG_RST_GPIO_Port, MAG_RST_Pin},     /* BSP_GPIO_MAG_RST */
    {KEY_GPIO_Port, KEY_Pin},             /* BSP_GPIO_KEY */
};

BSP_State BSP_GPIO_Init(BSP_GPIO *bsp_gpio, BSP_GPIO_ID id) {
  if (bsp_gpio == NULL || id >= BSP_GPIO_NUM) {
    return BSP_ERROR;
  }

  bsp_gpio->port = bsp_gpio_map[id].port;
  bsp_gpio->pin = bsp_gpio_map[id].pin;
  return BSP_OK;
}

void BSP_GPIO_WritePin(BSP_GPIO *bsp_gpio, GPIO_PinState state) {
  if (bsp_gpio == NULL || bsp_gpio->port == NULL) {
    return;
  }
  HAL_GPIO_WritePin(bsp_gpio->port, bsp_gpio->pin, state);
}

void BSP_GPIO_TogglePin(BSP_GPIO *bsp_gpio) {
  if (bsp_gpio == NULL || bsp_gpio->port == NULL) {
    return;
  }
  HAL_GPIO_TogglePin(bsp_gpio->port, bsp_gpio->pin);
}

GPIO_PinState BSP_GPIO_ReadPin(BSP_GPIO *bsp_gpio) {
  if (bsp_gpio == NULL || bsp_gpio->port == NULL) {
    return GPIO_PIN_RESET;
  }
  return HAL_GPIO_ReadPin(bsp_gpio->port, bsp_gpio->pin);
}
