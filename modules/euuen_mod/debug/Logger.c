#include "Logger.h"
#include <c_board_bsp/usart.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static float just_float_data[MAX_JUST_FLOAT_NUM + 1]; // 加一是为了存储尾帧
static uint8_t just_float_tail[4] = {0x00, 0x00, 0x80, 0x7f};

void Logger_Init(Logger *logger, BSP_USART *bsp_usart) {
  logger->bsp_usart = bsp_usart;
}

void Logger_Log(Logger *logger, const char *message) {
  BSP_USART_SendStr_DMA(logger->bsp_usart, message);
}

LoggerState Logger_Log_JustFloat(Logger *logger, const float *data, size_t length) {
    if (length > MAX_JUST_FLOAT_NUM) {
        return LOGGER_JUST_FLOAT_TOO_MUCH_DATA;
    }

    memcpy(just_float_data, data, length * sizeof(float));
    memcpy(&just_float_data[length], just_float_tail, 4);

    BSP_USART_SendData_DMA(logger->bsp_usart, (uint8_t*)just_float_data, length * sizeof(float) + 4);
    return LOGGER_OK;
}