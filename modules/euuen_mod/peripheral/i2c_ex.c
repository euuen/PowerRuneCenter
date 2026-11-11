#include <euuen_mod/debug/Logger.h>
#include <euuen_mod/interface/i2c.h>
#include <euuen_mod/interface/sys.h>
#include <euuen_mod/peripheral/i2c_ex.h>
#include <stddef.h>
#include <stdint.h>

/* ============================ 硬件 I2C 锁 ============================ */

/* 锁的是硬件 I2C（不是 ex 实例）：同一硬件 I2C 上的多个实例共用一把锁 */
static EM_MutexId s_hw_lock[EM_I2C_NUM];

static EM_MutexId EM_I2C_Ex_HwLock(EM_I2C_ID i2c) {
  if (i2c < 0 || i2c >= EM_I2C_NUM) {
    return NULL;
  }
  return s_hw_lock[i2c];
}

/* ============================ 公共 API ============================ */

void EM_I2C_Ex_Init(EM_I2C_Ex_t *self, EM_I2C_ID i2c, uint8_t dev_addr) {
  if (self == NULL || i2c < 0 || i2c >= EM_I2C_NUM) {
    return;
  }

  self->i2c = i2c;
  self->dev_addr = dev_addr;

  /* 硬件 I2C 初始化（接口自带防重） */
  EM_I2C_Init(i2c);

  /* 该硬件 I2C 的锁：首次创建，多个实例共用 */
  if (s_hw_lock[i2c] == NULL) {
    s_hw_lock[i2c] = EM_MutexCreate();
    if (s_hw_lock[i2c] == NULL) {
      LOG_ERROR("[EM_I2C_Ex_Init] i2c %d hw mutex create failed", (int)i2c);
    }
  }
}

uint8_t EM_I2C_Ex_Transmit(EM_I2C_Ex_t *self, const uint8_t *data,
                           uint16_t length) {
  if (self == NULL || data == NULL || length == 0) {
    return 0;
  }
  EM_MutexId lock = EM_I2C_Ex_HwLock(self->i2c);
  if (lock == NULL) {
    return 0;
  }

  EM_MutexTake(lock);
  uint8_t ok = EM_I2C_Transmit(self->i2c, self->dev_addr, data, length);
  EM_MutexGive(lock);
  return ok;
}

uint8_t EM_I2C_Ex_Receive(EM_I2C_Ex_t *self, uint8_t *data, uint16_t length) {
  if (self == NULL || data == NULL || length == 0) {
    return 0;
  }
  EM_MutexId lock = EM_I2C_Ex_HwLock(self->i2c);
  if (lock == NULL) {
    return 0;
  }

  EM_MutexTake(lock);
  uint8_t ok = EM_I2C_Receive(self->i2c, self->dev_addr, data, length);
  EM_MutexGive(lock);
  return ok;
}

uint8_t EM_I2C_Ex_MemWrite(EM_I2C_Ex_t *self, uint16_t mem_addr,
                           EM_I2C_MemAddrSize mem_size, const uint8_t *data,
                           uint16_t length) {
  if (self == NULL || data == NULL || length == 0) {
    return 0;
  }
  EM_MutexId lock = EM_I2C_Ex_HwLock(self->i2c);
  if (lock == NULL) {
    return 0;
  }

  EM_MutexTake(lock);
  uint8_t ok = EM_I2C_MemWrite(self->i2c, self->dev_addr, mem_addr, mem_size,
                               data, length);
  EM_MutexGive(lock);
  return ok;
}

uint8_t EM_I2C_Ex_MemRead(EM_I2C_Ex_t *self, uint16_t mem_addr,
                          EM_I2C_MemAddrSize mem_size, uint8_t *data,
                          uint16_t length) {
  if (self == NULL || data == NULL || length == 0) {
    return 0;
  }
  EM_MutexId lock = EM_I2C_Ex_HwLock(self->i2c);
  if (lock == NULL) {
    return 0;
  }

  EM_MutexTake(lock);
  uint8_t ok = EM_I2C_MemRead(self->i2c, self->dev_addr, mem_addr, mem_size,
                              data, length);
  EM_MutexGive(lock);
  return ok;
}
