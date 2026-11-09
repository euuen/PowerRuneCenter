#pragma once

#include <c_board_bsp/can.h>
#include <stdint.h>

#define BOARD_COUNT 5

typedef struct {
    uint8_t board_state;
    uint8_t score;
    uint8_t online;
    uint32_t last_tick;
} BoardInfo;

void Protocol_Init(BSP_CAN *can);
void Protocol_Process(void);
void Protocol_SendRequest(uint8_t board_idx, uint8_t color, uint8_t request_code,
                          uint8_t brightness);
void Protocol_SendAllRequests(uint8_t color, uint8_t request_code,
                              uint8_t brightness);
BoardInfo *Protocol_GetBoardInfo(uint8_t board_idx);
void Protocol_RxCallback(uint32_t std_id, uint8_t *data, uint8_t len);