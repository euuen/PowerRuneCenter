#include "protocol.h"
#include "protocol_type.h"
#include <c_board_bsp/sys.h>
#include <string.h>

#define BOARD_ONLINE_TIMEOUT_MS 50

static BSP_CAN *s_can = NULL;
static BoardInfo s_boards_info[BOARD_COUNT];

static const uint32_t s_board_state_ids[BOARD_COUNT] = {
    BOARD_1_STATE_CAN_ID, BOARD_2_STATE_CAN_ID, BOARD_3_STATE_CAN_ID,
    BOARD_4_STATE_CAN_ID, BOARD_5_STATE_CAN_ID
};

static const uint32_t s_center_request_ids[BOARD_COUNT] = {
    CENTER_REQUEST_1_CAN_ID, CENTER_REQUEST_2_CAN_ID, CENTER_REQUEST_3_CAN_ID,
    CENTER_REQUEST_4_CAN_ID, CENTER_REQUEST_5_CAN_ID
};


void Protocol_RxCallback(uint32_t std_id, uint8_t *data, uint8_t len) {
    if (len < sizeof(BoardStateFrame)) return;

    for (uint8_t i = 0; i < BOARD_COUNT; i++) {
        if (std_id == s_board_state_ids[i]) {
            BoardStateFrame *frame = (BoardStateFrame *)data;
            s_boards_info[i].board_state = frame->board_state;
            s_boards_info[i].score = frame->score;
            s_boards_info[i].online = 1;
            s_boards_info[i].last_tick = (uint32_t)BSP_GetTimestamp_ms_ISR();
            break;
        }
    }
}

void Protocol_Init(BSP_CAN *can) {
    s_can = can;
    memset(s_boards_info, 0, sizeof(s_boards_info));
    for (uint8_t i = 0; i < BOARD_COUNT; i++) {
        s_boards_info[i].board_state = BOARD_STATE_OFFLINE;
        s_boards_info[i].online = 0;
        s_boards_info[i].last_tick = 0;
    }
}

void Protocol_Process(void) {
    uint32_t now = (uint32_t)BSP_GetTimestamp_ms();
    for (uint8_t i = 0; i < BOARD_COUNT; i++) {
        if (s_boards_info[i].online &&
            (now - s_boards_info[i].last_tick) >= BOARD_ONLINE_TIMEOUT_MS) {
            s_boards_info[i].online = 0;
            s_boards_info[i].board_state = BOARD_STATE_OFFLINE;
        }
    }
}

void Protocol_SendRequest(uint8_t board_idx, uint8_t color, uint8_t request_code,
                          uint8_t brightness) {
    if (board_idx >= BOARD_COUNT || s_can == NULL) return;

    CenterRequestFrame frame;
    frame.target_color = color;
    frame.request_code = request_code;
    frame.brightness = brightness;

    BSP_CAN_SendStdFrame(s_can, s_center_request_ids[board_idx], (uint8_t *)&frame, sizeof(frame), 1.0f);
}

void Protocol_SendAllRequests(uint8_t color, uint8_t request_code,
                              uint8_t brightness) {
    for (uint8_t i = 0; i < BOARD_COUNT; i++) {
        Protocol_SendRequest(i, color, request_code, brightness);
    }
}

BoardInfo *Protocol_GetBoardInfo(uint8_t board_idx) {
    if (board_idx >= BOARD_COUNT) return NULL;
    return &s_boards_info[board_idx];
}
