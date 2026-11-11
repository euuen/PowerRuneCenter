#include "spi.h"
#include "type.h"
#include <spi.h>
#include <stdint.h>

static uint8_t check_spi_inited = 0;

/* HAL 回调里需要拿到实例，SPI1 只有一个实例 */
static BSP_SPI *bsp_spi_1_instance = NULL;

BSP_State BSP_SPI_1_Init(BSP_SPI *bsp_spi) {
  if (bsp_spi == NULL) {
    return BSP_ERROR;
  }
  if (bsp_spi->hspi != NULL) {
    return BSP_OK;
  }

  if (!check_spi_inited) {
    MX_SPI1_Init();
    check_spi_inited = 1;
  }

  bsp_spi->hspi = &hspi1;
  bsp_spi->busy = 0;
  bsp_spi_1_instance = bsp_spi;
  return BSP_OK;
}

BSP_State BSP_SPI_SendReceive(BSP_SPI *bsp_spi, uint8_t *tx_buf,
                              uint8_t *rx_buf, uint16_t length) {
  if (bsp_spi == NULL || bsp_spi->hspi == NULL || tx_buf == NULL ||
      rx_buf == NULL || length == 0) {
    return BSP_ERROR;
  }

  HAL_StatusTypeDef st = HAL_SPI_TransmitReceive(bsp_spi->hspi, tx_buf, rx_buf,
                                                 length, BSP_SPI_TIMEOUT_MS);
  if (st == HAL_OK) {
    return BSP_OK;
  }
  return (st == HAL_TIMEOUT) ? BSP_TIMEOUT : BSP_ERROR;
}

BSP_State BSP_SPI_SendReceiveDMA(BSP_SPI *bsp_spi, uint8_t *tx_buf,
                                 uint8_t *rx_buf, uint16_t length) {
  if (bsp_spi == NULL || bsp_spi->hspi == NULL || tx_buf == NULL ||
      rx_buf == NULL || length == 0) {
    return BSP_ERROR;
  }
  if (bsp_spi->busy) {
    return BSP_ERROR;
  }

  bsp_spi->busy = 1;
  if (HAL_SPI_TransmitReceive_DMA(bsp_spi->hspi, tx_buf, rx_buf, length) !=
      HAL_OK) {
    bsp_spi->busy = 0;
    return BSP_ERROR;
  }
  return BSP_OK;
}

uint8_t BSP_SPI_IsReady(BSP_SPI *bsp_spi) {
  if (bsp_spi == NULL || bsp_spi->hspi == NULL) {
    return 1;
  }
  return (bsp_spi->busy == 0);
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
  if (hspi == &hspi1 && bsp_spi_1_instance != NULL) {
    bsp_spi_1_instance->busy = 0;
  }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
  if (hspi == &hspi1 && bsp_spi_1_instance != NULL) {
    bsp_spi_1_instance->busy = 0;
  }
}
