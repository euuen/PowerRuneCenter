#include "i2c.h"
#include "type.h"
#include <i2c.h>
#include <stdint.h>

static uint8_t check_i2c3_inited = 0;

/* HAL 结果 -> BSP_State */
static BSP_State BSP_I2C_Report(HAL_StatusTypeDef st) {
  if (st == HAL_OK) {
    return BSP_OK;
  }
  return (st == HAL_TIMEOUT) ? BSP_TIMEOUT : BSP_ERROR;
}

BSP_State BSP_I2C_3_Init(BSP_I2C *bsp_i2c) {
  if (bsp_i2c == NULL) {
    return BSP_ERROR;
  }
  if (bsp_i2c->hi2c != NULL) {
    return BSP_OK;
  }

  if (!check_i2c3_inited) {
    MX_I2C3_Init();
    check_i2c3_inited = 1;
  }

  bsp_i2c->hi2c = &hi2c3;
  return BSP_OK;
}

BSP_State BSP_I2C_Transmit(BSP_I2C *bsp_i2c, uint8_t dev_addr,
                           const uint8_t *data, uint16_t length) {
  if (bsp_i2c == NULL || bsp_i2c->hi2c == NULL || data == NULL ||
      length == 0) {
    return BSP_ERROR;
  }
  return BSP_I2C_Report(HAL_I2C_Master_Transmit(
      bsp_i2c->hi2c, (uint16_t)(dev_addr << 1), (uint8_t *)data, length,
      BSP_I2C_TIMEOUT_MS));
}

BSP_State BSP_I2C_Receive(BSP_I2C *bsp_i2c, uint8_t dev_addr, uint8_t *data,
                          uint16_t length) {
  if (bsp_i2c == NULL || bsp_i2c->hi2c == NULL || data == NULL ||
      length == 0) {
    return BSP_ERROR;
  }
  return BSP_I2C_Report(HAL_I2C_Master_Receive(
      bsp_i2c->hi2c, (uint16_t)(dev_addr << 1), data, length,
      BSP_I2C_TIMEOUT_MS));
}

BSP_State BSP_I2C_MemWrite(BSP_I2C *bsp_i2c, uint8_t dev_addr,
                           uint16_t mem_addr, uint16_t mem_addr_size,
                           const uint8_t *data, uint16_t length) {
  if (bsp_i2c == NULL || bsp_i2c->hi2c == NULL || data == NULL ||
      length == 0) {
    return BSP_ERROR;
  }
  return BSP_I2C_Report(HAL_I2C_Mem_Write(
      bsp_i2c->hi2c, (uint16_t)(dev_addr << 1), mem_addr, mem_addr_size,
      (uint8_t *)data, length, BSP_I2C_TIMEOUT_MS));
}

BSP_State BSP_I2C_MemRead(BSP_I2C *bsp_i2c, uint8_t dev_addr, uint16_t mem_addr,
                          uint16_t mem_addr_size, uint8_t *data,
                          uint16_t length) {
  if (bsp_i2c == NULL || bsp_i2c->hi2c == NULL || data == NULL ||
      length == 0) {
    return BSP_ERROR;
  }
  return BSP_I2C_Report(HAL_I2C_Mem_Read(
      bsp_i2c->hi2c, (uint16_t)(dev_addr << 1), mem_addr, mem_addr_size, data,
      length, BSP_I2C_TIMEOUT_MS));
}
