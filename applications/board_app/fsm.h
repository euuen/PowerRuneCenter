#pragma once

typedef enum {
  CENTER_STATE_INACTIVE,     // 不可激活状态
  CENTER_STATE_ACTIVATING,   // 正在激活，按随机顺序等待灯板被击中
  CENTER_STATE_ACTIVE,       // 成功激活
} CenterState;


void FSM_Init();
void FSM_Process();