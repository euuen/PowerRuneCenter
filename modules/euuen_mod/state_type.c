#include "state_type.h"
#include <euuen_mod/debug/Logger.h>

const char* EM_State_ConvertStrTable[] = {
    "EUUEN_OK",
    "EUUEN_ERROR",
    "EUUEN_TIMEOUT",
    "EUUEN_NOT_INITIALIZED",
    "EUUEN_OUT_OF_MEMORY",
};

const char* EM_Module_ConvertStrTable[] = {
    "PID",
    "DJI_M3508",
    "MEM_ALLOCATOR",
};

const char* PID_ConvertStrTable[] = {
    "P",
    "PI",
    "PID",
    "P_LPF",
    "PI_LPF",
    "PID_LPF",
};

const char* DJI_M3508_ConvertStrTable[] = {
    "M1",
    "M2",
    "M3",
    "M4",
    "M5",
    "M6",
    "M7",
    "M8",
};

const char* MEM_ALLOCATOR_ConvertStrTable[] = {
    "RAM",
    "CCMRAM",
};

__attribute__((weak))
void EM_State_Handler(EM_Module module, uint8_t id,
                            EM_State state) {
    const char *module_str = (module < EUUEN_MODULE_COUNT)
                                 ? EM_Module_ConvertStrTable[module]
                                 : "?";
    const char *id_str = "?";
    switch (module) {
    case EUUEN_MODULE_PID:
        if (id < 6) {
            id_str = PID_ConvertStrTable[id];
        }
        break;
    case EUUEN_MODULE_DJI_M3508:
        if (id < 8) {
            id_str = DJI_M3508_ConvertStrTable[id];
        }
        break;
    case EUUEN_MODULE_MEM_ALLOCATOR:
        if (id < 2) {
            id_str = MEM_ALLOCATOR_ConvertStrTable[id];
        }
        break;
    default:
        break;
    }
    const char *state_str = (state < EUUEN_STATE_COUNT)
                                ? EM_State_ConvertStrTable[state]
                                : "?";

    /* LOG_* 兼容宏当前为空实现，显式消掉未使用告警 */
    (void)module_str;
    (void)id_str;
    (void)state_str;

    /* OK -> INFO 上报；异常 -> ERROR 上报后挂死，便于立即抓问题位置 */
    if (state == EUUEN_OK) {
        LOG_INFO("[%s %s] %s", module_str, id_str, state_str);
    } else {
        LOG_ERROR("[%s %s] %s", module_str, id_str, state_str);
        for (;;);
    }
}