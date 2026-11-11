#include <c_board_bsp/spi.h>
#include <euuen_mod/debug/Logger.h>
#include <euuen_mod/interface/spi.h>
#include <stddef.h>
#include <stdint.h>

/* ---- 驱动状态：每个 EM_SPI_ID 对应一个 BSP_SPI 实例，首次使用时绑定 ---- */

static BSP_SPI s_spi[EM_SPI_NUM];
static uint8_t s_spi_inited[EM_SPI_NUM];

static BSP_SPI *EM_SPI_Get(EM_SPI_ID spi) {
  if (spi < 0 || spi >= EM_SPI_NUM) {
    return NULL;
  }
  if (!s_spi_inited[spi]) {
    switch (spi) {
    case EM_SPI_1:
      if (BSP_SPI_1_Init(&s_spi[spi]) != BSP_OK) {
        return NULL;
      }
      break;
    default:
      return NULL;
    }
    s_spi_inited[spi] = 1;
  }
  return &s_spi[spi];
}

/* ---- 驱动 API ---- */

void EM_SPI_Init(EM_SPI_ID spi) {
  if (spi < 0 || spi >= EM_SPI_NUM) {
    return;
  }
  if (s_spi_inited[spi]) {
    LOG_WARN("[EM_SPI_Init] spi %d already initialized, skipped", (int)spi);
    return;
  }
  (void)EM_SPI_Get(spi);
}

uint8_t EM_SPI_SendReceive(EM_SPI_ID spi, uint8_t *tx, uint8_t *rx,
                           uint16_t length) {
  if (tx == NULL || rx == NULL || length == 0) {
    return 0;
  }
  BSP_SPI *bsp_spi = EM_SPI_Get(spi);
  if (bsp_spi == NULL) {
    return 0;
  }
  return (BSP_SPI_SendReceive(bsp_spi, tx, rx, length) == BSP_OK) ? 1 : 0;
}

uint8_t EM_SPI_SendReceiveDMA(EM_SPI_ID spi, uint8_t *tx, uint8_t *rx,
                              uint16_t length) {
  if (tx == NULL || rx == NULL || length == 0) {
    return 0;
  }
  BSP_SPI *bsp_spi = EM_SPI_Get(spi);
  if (bsp_spi == NULL) {
    return 0;
  }
  return (BSP_SPI_SendReceiveDMA(bsp_spi, tx, rx, length) == BSP_OK) ? 1 : 0;
}

uint8_t EM_SPI_IsReady(EM_SPI_ID spi) {
  BSP_SPI *bsp_spi = EM_SPI_Get(spi);
  if (bsp_spi == NULL) {
    return 0;
  }
  return BSP_SPI_IsReady(bsp_spi);
}
