#include <euuen_mod/debug/Logger.h>
#include <euuen_mod/interface/gpio.h>
#include <euuen_mod/interface/spi.h>
#include <euuen_mod/interface/sys.h>
#include <euuen_mod/peripheral/spi_ex.h>
#include <stddef.h>
#include <stdint.h>

/* ============================ 硬件 SPI 锁 ============================ */

/* 锁的是硬件 SPI（不是 ex 实例）：同一硬件 SPI 上的多个实例共用一把锁 */
static EM_MutexId s_hw_lock[EM_SPI_NUM];

static EM_MutexId EM_SPI_Ex_HwLock(EM_SPI_ID spi) {
  if (spi < 0 || spi >= EM_SPI_NUM) {
    return NULL;
  }
  return s_hw_lock[spi];
}

/* ============================ 片选 ============================ */

static void EM_SPI_Ex_CsLow(const EM_SPI_Ex_t *self) {
  if (self->cs != EM_GPIO_NONE) {
    EM_GPIO_WritePin(self->cs, EM_GPIO_RESET);
  }
}

static void EM_SPI_Ex_CsHigh(const EM_SPI_Ex_t *self) {
  if (self->cs != EM_GPIO_NONE) {
    EM_GPIO_WritePin(self->cs, EM_GPIO_SET);
  }
}

/* ============================ 公共 API ============================ */

void EM_SPI_Ex_Init(EM_SPI_Ex_t *self, EM_SPI_ID spi, EM_GPIO_ID cs) {
  if (self == NULL || spi < 0 || spi >= EM_SPI_NUM) {
    return;
  }

  self->spi = spi;
  self->cs = cs;

  /* 硬件 SPI 初始化（接口自带防重） */
  EM_SPI_Init(spi);

  /* 该硬件 SPI 的锁：首次创建，多个实例共用 */
  if (s_hw_lock[spi] == NULL) {
    s_hw_lock[spi] = EM_MutexCreate();
    if (s_hw_lock[spi] == NULL) {
      LOG_ERROR("[EM_SPI_Ex_Init] spi %d hw mutex create failed", (int)spi);
    }
  }
}

uint8_t EM_SPI_Ex_SendReceive(EM_SPI_Ex_t *self, uint8_t *tx, uint8_t *rx,
                              uint16_t length) {
  if (self == NULL || tx == NULL || rx == NULL || length == 0) {
    return 0;
  }
  EM_MutexId lock = EM_SPI_Ex_HwLock(self->spi);
  if (lock == NULL) {
    return 0;
  }

  EM_MutexTake(lock);
  EM_SPI_Ex_CsLow(self);
  uint8_t ok = EM_SPI_SendReceive(self->spi, tx, rx, length);
  EM_SPI_Ex_CsHigh(self);
  EM_MutexGive(lock);
  return ok;
}

uint8_t EM_SPI_Ex_SendReceiveDMA(EM_SPI_Ex_t *self, uint8_t *tx, uint8_t *rx,
                                 uint16_t length) {
  if (self == NULL || tx == NULL || rx == NULL || length == 0) {
    return 0;
  }
  EM_MutexId lock = EM_SPI_Ex_HwLock(self->spi);
  if (lock == NULL) {
    return 0;
  }

  EM_MutexTake(lock);
  EM_SPI_Ex_CsLow(self);
  uint8_t ok = EM_SPI_SendReceiveDMA(self->spi, tx, rx, length);
  if (ok) {
    while (!EM_SPI_IsReady(self->spi)) {
    }
  }
  EM_SPI_Ex_CsHigh(self);
  EM_MutexGive(lock);
  return ok;
}
