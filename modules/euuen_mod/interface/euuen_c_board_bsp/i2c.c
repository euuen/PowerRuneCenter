#include <c_board_bsp/i2c.h>
#include <euuen_mod/debug/Logger.h>
#include <euuen_mod/interface/i2c.h>
#include <i2c.h>
#include <stddef.h>
#include <stdint.h>

/* ---- 驱动状态：每个 EM_I2C_ID 对应一个 BSP_I2C 实例，首次使用时绑定 ---- */

static BSP_I2C s_i2c[EM_I2C_NUM];
static uint8_t s_i2c_inited[EM_I2C_NUM];

static BSP_I2C *EM_I2C_Get(EM_I2C_ID i2c) {
  if (i2c < 0 || i2c >= EM_I2C_NUM) {
    return NULL;
  }
  if (!s_i2c_inited[i2c]) {
    switch (i2c) {
    case EM_I2C_3:
      if (BSP_I2C_3_Init(&s_i2c[i2c]) != BSP_OK) {
        return NULL;
      }
      break;
    default:
      return NULL;
    }
    s_i2c_inited[i2c] = 1;
  }
  return &s_i2c[i2c];
}

static uint16_t EM_I2C_BspMemSize(EM_I2C_MemAddrSize mem_size) {
  return (mem_size == EM_I2C_MEMADD_16BIT) ? I2C_MEMADD_SIZE_16BIT
                                           : I2C_MEMADD_SIZE_8BIT;
}

/* ---- 驱动 API ---- */

void EM_I2C_Init(EM_I2C_ID i2c) {
  if (i2c < 0 || i2c >= EM_I2C_NUM) {
    return;
  }
  if (s_i2c_inited[i2c]) {
    LOG_WARN("[EM_I2C_Init] i2c %d already initialized, skipped", (int)i2c);
    return;
  }
  (void)EM_I2C_Get(i2c);
}

uint8_t EM_I2C_Transmit(EM_I2C_ID i2c, uint8_t dev_addr, const uint8_t *data,
                        uint16_t length) {
  if (data == NULL || length == 0) {
    return 0;
  }
  BSP_I2C *bsp_i2c = EM_I2C_Get(i2c);
  if (bsp_i2c == NULL) {
    return 0;
  }
  return (BSP_I2C_Transmit(bsp_i2c, dev_addr, data, length) == BSP_OK) ? 1 : 0;
}

uint8_t EM_I2C_Receive(EM_I2C_ID i2c, uint8_t dev_addr, uint8_t *data,
                       uint16_t length) {
  if (data == NULL || length == 0) {
    return 0;
  }
  BSP_I2C *bsp_i2c = EM_I2C_Get(i2c);
  if (bsp_i2c == NULL) {
    return 0;
  }
  return (BSP_I2C_Receive(bsp_i2c, dev_addr, data, length) == BSP_OK) ? 1 : 0;
}

uint8_t EM_I2C_MemWrite(EM_I2C_ID i2c, uint8_t dev_addr, uint16_t mem_addr,
                        EM_I2C_MemAddrSize mem_size, const uint8_t *data,
                        uint16_t length) {
  if (data == NULL || length == 0) {
    return 0;
  }
  BSP_I2C *bsp_i2c = EM_I2C_Get(i2c);
  if (bsp_i2c == NULL) {
    return 0;
  }
  return (BSP_I2C_MemWrite(bsp_i2c, dev_addr, mem_addr,
                           EM_I2C_BspMemSize(mem_size), data,
                           length) == BSP_OK)
             ? 1
             : 0;
}

uint8_t EM_I2C_MemRead(EM_I2C_ID i2c, uint8_t dev_addr, uint16_t mem_addr,
                       EM_I2C_MemAddrSize mem_size, uint8_t *data,
                       uint16_t length) {
  if (data == NULL || length == 0) {
    return 0;
  }
  BSP_I2C *bsp_i2c = EM_I2C_Get(i2c);
  if (bsp_i2c == NULL) {
    return 0;
  }
  return (BSP_I2C_MemRead(bsp_i2c, dev_addr, mem_addr,
                          EM_I2C_BspMemSize(mem_size), data, length) == BSP_OK)
             ? 1
             : 0;
}
