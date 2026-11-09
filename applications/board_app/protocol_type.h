#pragma once

#include <stdint.h>

#pragma pack(1)

typedef struct {
    uint8_t board_state;
    uint8_t score;
} BoardStateFrame;

typedef struct {
    uint8_t target_color;
    uint8_t request_code;
    uint8_t brightness;
} CenterRequestFrame;

#pragma pack()

typedef enum {
    BOARD_1_STATE_CAN_ID = 0x101,
    BOARD_2_STATE_CAN_ID = 0x102,
    BOARD_3_STATE_CAN_ID = 0x103,
    BOARD_4_STATE_CAN_ID = 0x104,
    BOARD_5_STATE_CAN_ID = 0x105,

    CENTER_REQUEST_1_CAN_ID = 0x301,
    CENTER_REQUEST_2_CAN_ID = 0x302,
    CENTER_REQUEST_3_CAN_ID = 0x303,
    CENTER_REQUEST_4_CAN_ID = 0x304,
    CENTER_REQUEST_5_CAN_ID = 0x305,

    M3508_CMD_CAN_ID = 0x200,
    M3508_FEEDBACK_CAN_ID = 0x201,
} Protocol_CanID;

typedef enum {
    CENTER_REQUEST_NOTHING = 0,
    CENTER_REQUEST_READY,
    CENTER_REQUEST_RESET,
} CenterRequestCode;

typedef enum {
    COLOR_BLUE = 0,
    COLOR_RED = 1,
} TargetColor;

typedef enum {
    BOARD_STATE_READY = 0,
    BOARD_STATE_HIT,
    BOARD_STATE_TIMEOUT, // 保留占位，超时由中心板计算
    BOARD_STATE_IGNORE,
    BOARD_STATE_OFFLINE,
} BoardState;
