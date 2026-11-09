#include "fsm.h"
#include "protocol.h"
#include <c_board_bsp/can.h>
#include <c_board_bsp/sys.h>
#include <c_board_bsp/usart.h>
#include <cmsis_os.h>
#include <euuen_mod/debug/Logger.h>
#include <euuen_mod/motor/DJI_M3508.h>
#include <euuen_mod/remote_control/SBUS_RC.h>

BSP_CAN bsp_can_1;
BSP_USART bsp_usart_1;
Logger logger;
// DJI_M3508 dji_m3508;

void APP_Setup() {
  BSP_CAN_1_Init(&bsp_can_1);
  BSP_CAN_1_AddRxCallback(Protocol_RxCallback);
  // BSP_CAN_1_AddRxCallback(DJI_M3508_RxCallback);
  BSP_USART_1_Init(&bsp_usart_1);

  Logger_Init(&logger, &bsp_usart_1);
  // DJI_M3508_Init(&dji_m3508, &bsp_can_1, 1);
  Protocol_Init(&bsp_can_1);
  FSM_Init();
  Logger_Log(&logger, "center inited over");
}

// TODO: 旋转M3508
void APP_Loop() {
  FSM_Process();
  Protocol_Process();
  osDelay(5);
}
