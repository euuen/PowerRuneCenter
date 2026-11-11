#include <euuen_mod/debug/Logger.h>
#include <euuen_mod/euuen_mod.h>
#include <euuen_mod/interface/can.h>
#include <euuen_mod/interface/sys.h>
#include <euuen_mod/mem_allocator.h>
#include <euuen_mod/peripheral/can_ex.h>
#include <stddef.h>
#include <stdint.h>

/* ============================ Handler 链 ============================ */

typedef struct EM_CAN_Ex_RxHandlerNode_t {
  struct EM_CAN_Ex_RxHandlerNode_t *next;
  EM_CAN_Ex_RxHandler handler;
} EM_CAN_Ex_RxHandlerNode_t;

/* ============================ Ex 实例 ============================ */

typedef struct EM_CAN_Ex_t {
  EM_CAN_Ex_RxHandlerNode_t *rx_handler_chain;
  volatile uint8_t is_inited;
} EM_CAN_Ex_t;

/* 静态全局变量：按 EUUEN 枚举直接索引 */
static EM_CAN_Ex_t s_can_ex[EUUEN_CAN_NUM];

static EM_CAN_Ex_t *EM_CAN_Ex_Get(EM_CAN_ID can) {
  if (can < 0 || can >= EUUEN_CAN_NUM) {
    return NULL;
  }
  if (!s_can_ex[can].is_inited) {
    return NULL;
  }
  return &s_can_ex[can];
}

/* 驱动 RX 入口：遍历本层 handler 链分发 */
static void EM_CAN_Ex_OnRx(EM_CAN_ID can, uint32_t can_id, uint8_t *data,
                           uint8_t length) {
  EM_CAN_Ex_t *self = EM_CAN_Ex_Get(can);
  if (self == NULL) {
    return;
  }
  for (EM_CAN_Ex_RxHandlerNode_t *cur = self->rx_handler_chain; cur != NULL;
       cur = cur->next) {
    if (cur->handler != NULL) {
      cur->handler(can_id, data, length);
    }
  }
}

/* ============================ 公共 API ============================ */

void EM_CAN_Ex_Init(EM_CAN_ID can) {
  if (can < 0 || can >= EUUEN_CAN_NUM) {
    return;
  }

  EM_CAN_Ex_t *self = &s_can_ex[can];

  /* 防重复初始化 */
  if (self->is_inited) {
    LOG_WARN("[EM_CAN_Ex_Init] can %d already initialized, skipped", (int)can);
    return;
  }

  self->rx_handler_chain = NULL;
  EM_CAN_Init(can); /* interface 驱动初始化（自身也带防重复） */
  EM_CAN_SetRxHandler(can, EM_CAN_Ex_OnRx);
  self->is_inited = 1;
}

void EM_CAN_Ex_EnableRxHandler(EM_CAN_ID can) {
  EM_CAN_Ex_t *self = EM_CAN_Ex_Get(can);
  if (self == NULL) {
    return;
  }
  EM_CAN_SetRxHandler(can, EM_CAN_Ex_OnRx);
  EM_CAN_EnableRxHandler(can);
}

void EM_CAN_Ex_AddRxHandler(EM_CAN_ID can, EM_CAN_Ex_RxHandler handler) {
  if (handler == NULL) {
    return;
  }

  EM_CAN_Ex_t *self = EM_CAN_Ex_Get(can);
  if (self == NULL) {
    return;
  }

  EM_EnterCritical();

  /* 尾插，先到先注册优先被调用；顺便查重 */
  EM_CAN_Ex_RxHandlerNode_t *cur = self->rx_handler_chain;
  EM_CAN_Ex_RxHandlerNode_t *tail = NULL;
  while (cur != NULL) {
    if (cur->handler == handler) {
      EM_ExitCritical();
      return; /* 已注册，不重复添加 */
    }
    tail = cur;
    cur = cur->next;
  }

  EM_CAN_Ex_RxHandlerNode_t *node = (EM_CAN_Ex_RxHandlerNode_t *)
      MemAllocator_Alloc(&ram_allocator, sizeof(EM_CAN_Ex_RxHandlerNode_t));
  if (node == NULL) {
    EM_ExitCritical();
    return;
  }
  node->handler = handler;
  node->next = NULL;

  if (tail == NULL) {
    self->rx_handler_chain = node;
  } else {
    tail->next = node;
  }

  EM_ExitCritical();
}

void EM_CAN_Ex_SendStdFrame(EM_CAN_ID can, uint16_t can_id, const uint8_t *data,
                            size_t length, float timeout) {
  if (EM_CAN_Ex_Get(can) == NULL) {
    return;
  }
  EM_CAN_SendStdFrame(can, can_id, data, length, timeout);
}
