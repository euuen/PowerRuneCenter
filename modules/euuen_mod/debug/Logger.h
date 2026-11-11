#pragma once

#include <c_board_bsp/usart.h>
#include <stddef.h>

#define MAX_JUST_FLOAT_NUM 16


typedef enum {
    LOGGER_OK,
    LOGGER_ERROR,
    LOGGER_JUST_FLOAT_TOO_MUCH_DATA,
} LoggerState;

typedef struct {
    BSP_USART *bsp_usart;
} Logger;

void Logger_Init(Logger *logger, BSP_USART * bsp_usart);
void Logger_Log(Logger *logger, const char *message);
LoggerState Logger_Log_JustFloat(Logger *logger, const float *data, size_t length);

/* 旧框架日志宏兼容：当前阶段只保证编译，传感器等模块的错误日志暂不输出。
 * 后续迁移旧 Logger 时再替换为真实实现。 */
#define LOG_INFO(...) ((void)0)
#define LOG_WARN(...) ((void)0)
#define LOG_ERROR(...) ((void)0)
