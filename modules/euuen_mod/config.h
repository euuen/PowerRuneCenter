#pragma once

/* 内存堆 */
#define MEM_ALLOCATOR_RAM_HEAP_SIZE 20 * 1024
#define MEM_ALLOCATOR_CCMRAM_HEAP_SIZE 10 * 1024
#define MEM_ALLOCATOR_BLOCK_ON_OOM 1 /* 发生OOM的时候阻塞程序 */

/* 串口链式块缓冲：块大小、RX 块池全局上限、单口 TX 占用上限 */
#define USART_BLOCK_SIZE 256
#define USART_RX_MAX_BLOCKS 12 /* 全部串口 RX 共享的块数上限 */
#define USART_TX_MAX_BLOCKS 12 /* 单口 TX 同时占用的块数上限 */
#define USART_TX_MAX_WARN_BLOCKS 8 /* 单口 TX 同时待发的溢出告警块上限（防刷屏） */

/* peripheral/usart_ex：每端口 RX 块池大小（ping-pong 轮转，收完自动重 arm） */
#define USART_EX_RX_BLOCKS 2

/* Logger */
#define LOGGER_LINE_BUF 192 /* 单条日志格式化栈缓冲 */
#define LOGGER_STREAM  EUUEN_LOG_STREAM_USART_ID           /* 输出流：取 Logger.h                                               \
                                         的数值宏（EUUEN_LOG_STREAM_NONE_ID=0/USART_ID=1/SPI_ID=2），供 \
                                         #if 比较，不能用枚举名（#if 中会当 0 处理） */
#define LOGGER_USART_ID  EUUEN_USART_1 /* 用哪个 USART 输出日志 */
#define LOGGER_SPI_ID 0               /* 用哪个 SPI 输出日志（预留） */
#define LOGGER_DEFAULT_LEVEL  LOGGER_LEVEL_WARN /* 默认输出等级：LOGGER_LEVEL_INFO/WARN/ERROR/OFF */