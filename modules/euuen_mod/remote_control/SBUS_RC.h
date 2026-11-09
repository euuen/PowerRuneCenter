#pragma once

#include <stdint.h>

#define RC_OFFLINE_TIMEOUT_MS 100


typedef struct {
    uint16_t channels[16];
    uint32_t last_rx_tick;
} SBUS_RC;

extern SBUS_RC sbus_rc;

void SBUS_RC_Init();