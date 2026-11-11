#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* 其他组件的聚合头文件：当前聚合已迁移的 peripheral/sensor 依赖，
 * 后续迁移 PID/filter/VOFA/motor/remote 等模块时再继续补全。 */

#include <euuen_mod/interface.h>
#include <euuen_mod/peripheral/spi_ex.h>
#include <euuen_mod/peripheral/i2c_ex.h>
#include <euuen_mod/peripheral/can_ex.h>
#include <euuen_mod/peripheral/usart_ex.h>
#include <euuen_mod/peripheral/gpio.h>
#include <euuen_mod/state_type.h>
#include <euuen_mod/mem_allocator.h>
#include <euuen_mod/config.h>
#include <euuen_mod/debug/Logger.h>
#include <euuen_mod/sensor/bmi088.h>
#include <euuen_mod/sensor/ist8310.h>
#include <euuen_mod/sensor/mahony.h>

#include <stdint.h>

extern uint8_t ram_allocator_buffer[MEM_ALLOCATOR_RAM_HEAP_SIZE];
extern MemAllocator ram_allocator;
extern uint8_t ccmram_allocator_buffer[MEM_ALLOCATOR_CCMRAM_HEAP_SIZE];
extern MemAllocator ccmram_allocator;

void EM_MakeContext(void);

#ifdef __cplusplus
}
#endif
