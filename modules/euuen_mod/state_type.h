#pragma once

#include <stdint.h>

typedef enum {
    EUUEN_OK = 0,
    EUUEN_ERROR,
    EUUEN_TIMEOUT,
    EUUEN_NOT_INITIALIZED,
    EUUEN_OUT_OF_MEMORY,
    EUUEN_STATE_COUNT,
} EM_State;

extern const char* EM_State_ConvertStrTable[];
extern const char* EM_Module_ConvertStrTable[];
extern const char* PID_ConvertStrTable[];
extern const char* DJI_M3508_ConvertStrTable[];
extern const char* MEM_ALLOCATOR_ConvertStrTable[];

typedef enum {
    EUUEN_MODULE_PID = 0,
    EUUEN_MODULE_DJI_M3508,
    EUUEN_MODULE_MEM_ALLOCATOR,
    EUUEN_MODULE_COUNT,
} EM_Module;

/* euuen 模块状态回调（weak 实现，应用层可重新定义）：
 * 子模块在返回状态之前调用一次，统一汇总各小组件的运行状态。
 * module：子模块；id：子模块实例编号（表内下标）；state：即将返回的状态。
 * 默认实现：OK -> LOG_INFO；异常 -> LOG_ERROR 并 for(;;) 挂死。 */
void EM_State_Handler(EM_Module module, uint8_t id,
                            EM_State state);