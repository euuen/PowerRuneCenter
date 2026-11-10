#pragma once

#include <stdint.h>

typedef enum {
  FSi6x_STICK_STATE_UP,
  FSi6x_STICK_STATE_MID,
  FSi6x_STICK_STATE_DOWN,
} FSi6x_STICK_STATE;

// 通道顺序请按照下面结构体成员的顺序设置。

typedef struct {
  float right_stick_x; //-1 ~ 1
  float right_stick_y; //-1 ~ 1
  float left_stick_y;  // -1 ~ 1
  float left_stick_x;  //-1 ~ 1
  FSi6x_STICK_STATE swa;
  FSi6x_STICK_STATE swb;
  FSi6x_STICK_STATE swc;
  FSi6x_STICK_STATE swd;
  float vra; // 0 ~ 1
  float vrb; // 0 ~ 1
} FSi6x_Data;

void FSi6x_Init(FSi6x_Data *data);
void FSi6x_Read(FSi6x_Data *data);
uint8_t FSi6x_IsDisconnectd();