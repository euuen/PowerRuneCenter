#include "FSi6x.h"
#include "SBUS_RC.h"
#include <stdint.h>

// ---------- 模拟量范围 ----------
#define RIGHT_STICK_X_MIN 240
#define RIGHT_STICK_X_MAX 1807
#define RIGHT_STICK_Y_MIN 240
#define RIGHT_STICK_Y_MAX 1807
#define LEFT_STICK_X_MIN 240
#define LEFT_STICK_X_MAX 1807
#define LEFT_STICK_Y_MIN 240
#define LEFT_STICK_Y_MAX 1807
#define VRA_MIN 240
#define VRA_MAX 1807
#define VRB_MIN 240
#define VRB_MAX 1807

// ---------- 开关位置原始值 ----------
#define SWA_UP 240
#define SWA_DOWN 1807
#define SWB_UP 240
#define SWB_DOWN 1807
#define SWC_UP 240
#define SWC_MID 1024
#define SWC_DOWN 1807
#define SWD_UP 240
#define SWD_DOWN 1807

// ---------- 三态开关阈值 ----------
#define SWITCH_THRESHOLD_LOW 700
#define SWITCH_THRESHOLD_HIGH 1400

// ---------- 映射宏（直接内联，无函数） ----------
#define MAP_TO_01(raw, min, max) (((float)(raw) - (min)) / ((max) - (min)))
#define MAP_TO_NEG1_1(raw, min, max)                                           \
  (((float)(raw) - ((min) + (max)) / 2.0f) / (((max) - (min)) / 2.0f))

void FSi6x_Init(FSi6x_Data *data) {
  data->left_stick_x = 0;
  data->left_stick_y = 0;
  data->right_stick_x = 0;
  data->right_stick_y = 0;
  data->swa = FSi6x_STICK_STATE_UP;
  data->swb = FSi6x_STICK_STATE_UP;
  data->swc = FSi6x_STICK_STATE_UP;
  data->swd = FSi6x_STICK_STATE_UP;
  data->vra = 0;
  data->vrb = 0;
  SBUS_RC_Init();
}

void FSi6x_Read(FSi6x_Data *data) {
  // ---- 摇杆（映射到 -1 ~ 1） ----
  data->right_stick_x = MAP_TO_NEG1_1(
      SBUS_RC_GetChannel(SBUS_CHANNEL_1), RIGHT_STICK_X_MIN, RIGHT_STICK_X_MAX);
  data->right_stick_y = MAP_TO_NEG1_1(
      SBUS_RC_GetChannel(SBUS_CHANNEL_2), RIGHT_STICK_Y_MIN, RIGHT_STICK_Y_MAX);
  data->left_stick_y = MAP_TO_NEG1_1(
      SBUS_RC_GetChannel(SBUS_CHANNEL_3), LEFT_STICK_Y_MIN, LEFT_STICK_Y_MAX);
  data->left_stick_x = MAP_TO_NEG1_1(
      SBUS_RC_GetChannel(SBUS_CHANNEL_4), LEFT_STICK_X_MIN, LEFT_STICK_X_MAX);

  // ---- 两态开关：SWA, SWB, SWD ----
  data->swa = (SBUS_RC_GetChannel(SBUS_CHANNEL_5) > (SWA_UP + SWA_DOWN) / 2)
                  ? FSi6x_STICK_STATE_DOWN
                  : FSi6x_STICK_STATE_UP;

  data->swb = (SBUS_RC_GetChannel(SBUS_CHANNEL_6) > (SWB_UP + SWB_DOWN) / 2)
                  ? FSi6x_STICK_STATE_DOWN
                  : FSi6x_STICK_STATE_UP;

  {
    uint16_t raw = SBUS_RC_GetChannel(SBUS_CHANNEL_7);
    if (raw > SWITCH_THRESHOLD_HIGH)
      data->swc = FSi6x_STICK_STATE_DOWN;
    else if (raw < SWITCH_THRESHOLD_LOW)
      data->swc = FSi6x_STICK_STATE_UP;
    else
      data->swc = FSi6x_STICK_STATE_MID;
  }

  data->swd = (SBUS_RC_GetChannel(SBUS_CHANNEL_8) > (SWD_UP + SWD_DOWN) / 2)
                  ? FSi6x_STICK_STATE_DOWN
                  : FSi6x_STICK_STATE_UP;

  // ---- 旋钮（映射到 0 ~ 1） ----
  data->vra = MAP_TO_01(SBUS_RC_GetChannel(SBUS_CHANNEL_9), VRA_MIN, VRA_MAX);
  data->vrb = MAP_TO_01(SBUS_RC_GetChannel(SBUS_CHANNEL_10), VRB_MIN, VRB_MAX);
}

uint8_t FSi6x_IsDisconnectd() {
  // 当接收机接收超时，丢帧，有异常的时候判定断连
  return SBUS_RC_IsTimeout()
      || SBUS_RC_GetFlag(SBUS_LOST_FRAME)
      || SBUS_RC_GetFlag(SBUS_FAIL_SAFE);
}
