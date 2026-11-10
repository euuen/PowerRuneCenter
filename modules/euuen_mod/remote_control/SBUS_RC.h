#pragma once

#include <stdint.h>

#define RC_OFFLINE_TIMEOUT_MS 100

typedef enum {
    SBUS_CHANNEL_1,
    SBUS_CHANNEL_2,
    SBUS_CHANNEL_3,
    SBUS_CHANNEL_4,
    SBUS_CHANNEL_5,
    SBUS_CHANNEL_6,
    SBUS_CHANNEL_7,
    SBUS_CHANNEL_8,
    SBUS_CHANNEL_9,
    SBUS_CHANNEL_10,
    SBUS_CHANNEL_11,
    SBUS_CHANNEL_12,
    SBUS_CHANNEL_13,
    SBUS_CHANNEL_14,
    SBUS_CHANNEL_15,
    SBUS_CHANNEL_16,
    SBUS_CHANNEL_17,
    SBUS_CHANNEL_18,
} SBUS_Channel;

typedef enum {
    SBUS_LOST_FRAME,
    SBUS_FAIL_SAFE
} SBUS_Flag;

typedef struct {
    uint8_t data_frame[25];
    uint32_t last_rx_tick;
} SBUS_RC;

extern SBUS_RC sbus_rc;

void SBUS_RC_Init();
uint16_t SBUS_RC_GetChannel(SBUS_Channel channel_id);
uint8_t SBUS_RC_GetFlag(SBUS_Flag flag_id);
uint8_t SBUS_RC_IsTimeout(void);