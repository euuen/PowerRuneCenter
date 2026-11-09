#include "fsm.h"
#include "protocol.h"
#include "protocol_type.h"
#include "main.h"
#include <c_board_bsp/sys.h>
#include <cmsis_os.h>
#include <euuen_mod/remote_control/FSi6x.h>
#include <stdint.h>
#include <stdio.h>

#define FSM_BROADCAST_PERIOD_MS 50
#define ACTIVATE_TIMEOUT_MS 2500

CenterState center_state;

static FSi6x_Data fsi6x_data;
static uint8_t activate_order[BOARD_COUNT];
static uint8_t activate_remaining;
static uint32_t rand_state;
static uint32_t last_broadcast_tick;
static uint32_t activate_tick;

static uint32_t FSM_Random(void) {
  rand_state = rand_state * 1103515245u + 12345u;
  return rand_state;
}

static uint8_t FSM_GetTargetColor(void) {
  return fsi6x_data.swc == FSi6x_STICK_STATE_DOWN ? COLOR_RED : COLOR_BLUE;
}

static uint8_t FSM_GetBrightness(void) {
  float v = fsi6x_data.vrb;
  if (v < 0.0f) {
    v = 0.0f;
  } else if (v > 1.0f) {
    v = 1.0f;
  }
  return (uint8_t)(v * 255.0f + 0.5f);
}

// 周期性向全部灯板同步颜色和亮度，不改变灯板状态。
static void FSM_BroadcastIfDue(uint8_t color, uint8_t brightness) {
  uint32_t now = HAL_GetTick();
  if ((now - last_broadcast_tick) < FSM_BROADCAST_PERIOD_MS) return;
  last_broadcast_tick = now;
  Protocol_SendAllRequests(color, CENTER_REQUEST_NOTHING, brightness);
}

static void FSM_ExitState(CenterState _center_state) {
  switch (_center_state) {
  case CENTER_STATE_INACTIVE:
    break;
  case CENTER_STATE_ACTIVATING:
    break;
  case CENTER_STATE_ACTIVE:
    break;
  default:
    break;
  }
}

static void FSM_EnterState(CenterState _center_state) {
  switch (_center_state) {
  case CENTER_STATE_INACTIVE:
    Logger_Log(&logger, "enter inactive\n");
    break;
  case CENTER_STATE_ACTIVATING: {
    activate_remaining = 0;
    for (uint8_t i = 0; i < BOARD_COUNT; i++) {
      if (Protocol_GetBoardInfo(i)->board_state != BOARD_STATE_OFFLINE) {
        activate_order[activate_remaining++] = i;
      }else {
        char out_str[32];
        snprintf(out_str, sizeof(out_str), "board %u offline\n", i + 1);
        Logger_Log(&logger, out_str);
      }
    }
    for (uint8_t i = activate_remaining; i > 1; i--) {
      uint8_t j = (uint8_t)(FSM_Random() % i);
      uint8_t tmp = activate_order[i - 1];
      activate_order[i - 1] = activate_order[j];
      activate_order[j] = tmp;
    }
    Logger_Log(&logger, "enter activating\n");
    last_broadcast_tick = HAL_GetTick();
    activate_tick = HAL_GetTick();
    if (activate_remaining > 0) {
      uint8_t idx = activate_order[activate_remaining - 1];
      Protocol_SendRequest(idx, FSM_GetTargetColor(), CENTER_REQUEST_READY,
                           FSM_GetBrightness());
      char out_str[32];
      snprintf(out_str, sizeof(out_str), "waiting board %u hit\n", idx + 1);
      Logger_Log(&logger, out_str);
    }
    break;
  }
  case CENTER_STATE_ACTIVE:
    Logger_Log(&logger, "enter active\n");
    break;
  default:
    break;
  }
}

static void FSM_ChangeState(CenterState new_center_state) {
  FSM_ExitState(center_state);
  center_state = new_center_state;
  FSM_EnterState(new_center_state);
}

void FSM_Init() {
  rand_state = HAL_GetTick();
  FSi6x_Init(&fsi6x_data);
  center_state = CENTER_STATE_INACTIVE;
  last_broadcast_tick = HAL_GetTick();
}

void FSM_Process() {
  FSi6x_Read(&fsi6x_data);
  Protocol_Process();

  uint8_t color = FSM_GetTargetColor();
  uint8_t brightness = FSM_GetBrightness();
  BoardInfo *board_info;
  switch (center_state) {
  case CENTER_STATE_INACTIVE:
    Protocol_SendAllRequests(color, CENTER_REQUEST_RESET, brightness);
    if (fsi6x_data.swc != FSi6x_STICK_STATE_UP) {
      FSM_ChangeState(CENTER_STATE_ACTIVATING);
    }
    break;
  case CENTER_STATE_ACTIVATING:
    if (fsi6x_data.swc == FSi6x_STICK_STATE_UP) {
      FSM_ChangeState(CENTER_STATE_INACTIVE);
      break;
    }
    FSM_BroadcastIfDue(color, brightness);
    if (activate_remaining == 0) {
      FSM_ChangeState(CENTER_STATE_ACTIVE);
      break;
    }
    {
      uint8_t idx = activate_order[activate_remaining - 1];
      Protocol_SendRequest(idx, color, CENTER_REQUEST_READY, brightness);
      board_info = Protocol_GetBoardInfo(idx);
      if (board_info->board_state == BOARD_STATE_HIT) {
        activate_remaining--;
        char out_str[32];
        snprintf(out_str, sizeof(out_str), "board %u hit, %u left\n", idx + 1, activate_remaining);
        Logger_Log(&logger, out_str);
        if (activate_remaining > 0) {
          activate_tick = HAL_GetTick();
          uint8_t next_idx = activate_order[activate_remaining - 1];
          snprintf(out_str, sizeof(out_str), "waiting board %u hit\n", next_idx + 1);
          Logger_Log(&logger, out_str);
        }
      }else if (fsi6x_data.swd != FSi6x_STICK_STATE_DOWN &&
               (HAL_GetTick() - activate_tick) >= ACTIVATE_TIMEOUT_MS) {
        Logger_Log(&logger, "activate timeout\n");
        FSM_ChangeState(CENTER_STATE_INACTIVE);
      }
    }
    break;
  case CENTER_STATE_ACTIVE:
    if (fsi6x_data.swc == FSi6x_STICK_STATE_UP) {
      FSM_ChangeState(CENTER_STATE_INACTIVE);
      break;
    }
    FSM_BroadcastIfDue(color, brightness);
    break;
  default:
    break;
  }
}