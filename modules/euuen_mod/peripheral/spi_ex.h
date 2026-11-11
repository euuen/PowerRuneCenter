#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <euuen_mod/interface/gpio.h>
#include <euuen_mod/interface/spi.h>
#include <stdint.h>

/* SPI 实例：绑定一个硬件 SPI + 一个片选（EM_GPIO_NONE 表示不控制片选）。
 * 实例可内嵌在使用方结构体里（如 BMI088 的 accel/gyro 两个实例），
 * 收发时内部自动锁住硬件 SPI、拉/放片选。tx/rx 由调用方提供。 */
typedef struct EM_SPI_Ex_t {
  EM_SPI_ID spi;  /* 挂在哪个硬件 SPI */
  EM_GPIO_ID cs;  /* 本实例片选；EM_GPIO_NONE = 不控制 */
} EM_SPI_Ex_t;

void EM_SPI_Ex_Init(EM_SPI_Ex_t *self, EM_SPI_ID spi, EM_GPIO_ID cs);
/* 阻塞收发：锁硬件SPI -> CS低 -> EM_SPI_SendReceive -> CS高 -> 解锁；返回 1/0 */
uint8_t EM_SPI_Ex_SendReceive(EM_SPI_Ex_t *self, uint8_t *tx, uint8_t *rx,
                              uint16_t length);
/* DMA 收发：锁硬件SPI -> CS低 -> EM_SPI_SendReceiveDMA -> 等完成 -> CS高 -> 解锁；返回 1/0 */
uint8_t EM_SPI_Ex_SendReceiveDMA(EM_SPI_Ex_t *self, uint8_t *tx, uint8_t *rx,
                                 uint16_t length);

#ifdef __cplusplus
}
#endif
