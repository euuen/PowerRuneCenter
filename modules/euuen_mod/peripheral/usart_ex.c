#include <euuen_mod/config.h>
#include <euuen_mod/debug/Logger.h>
#include <euuen_mod/euuen_mod.h>
#include <euuen_mod/interface/sys.h>
#include <euuen_mod/interface/usart.h>
#include <euuen_mod/mem_allocator.h>
#include <euuen_mod/peripheral/usart_ex.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* ============================ TX 动态链表块 ============================ */

typedef struct EM_USART_Ex_TxBufferNode_t {
  struct EM_USART_Ex_TxBufferNode_t *next;
  uint8_t buffer[USART_BLOCK_SIZE];
  uint8_t *buffer_head;
  uint8_t *buffer_tail;
  uint8_t is_warn; /* 特殊块：溢出告警专用，不计入 tx_block_count */
} EM_USART_Ex_TxBufferNode_t;

static void EM_USART_Ex_TxBufferNode_Init(EM_USART_Ex_TxBufferNode_t *self) {
  if (self == NULL) {
    return;
  }
  self->buffer_head = &self->buffer[0];
  self->buffer_tail = self->buffer_head;
  self->next = NULL;
  self->is_warn = 0;
}

static size_t
EM_USART_Ex_TxBufferNode_GetUsedSize(EM_USART_Ex_TxBufferNode_t *self) {
  if (self == NULL) {
    return 0;
  }

  size_t head_off = (size_t)(self->buffer_head - self->buffer);
  size_t tail_off = (size_t)(self->buffer_tail - self->buffer);

  size_t used_size =
      (tail_off - head_off + USART_BLOCK_SIZE) % USART_BLOCK_SIZE;
  return used_size;
}

static size_t
EM_USART_Ex_TxBufferNode_GetFreeSize(EM_USART_Ex_TxBufferNode_t *self) {
  if (self == NULL) {
    return 0;
  }

  return USART_BLOCK_SIZE - 1 - EM_USART_Ex_TxBufferNode_GetUsedSize(self);
}

static size_t EM_USART_Ex_TxBufferNode_PutData(EM_USART_Ex_TxBufferNode_t *self,
                                               const uint8_t *data,
                                               size_t length) {
  if (self == NULL || data == NULL) {
    return length;
  }

  if (length == 0) {
    return 0;
  }

  size_t free_size = EM_USART_Ex_TxBufferNode_GetFreeSize(self);
  size_t write_len = (length < free_size) ? length : free_size;

  for (size_t i = 0; i < write_len; i++) {
    *self->buffer_tail = data[i];
    self->buffer_tail++;

    if (self->buffer_tail == &self->buffer[USART_BLOCK_SIZE]) {
      self->buffer_tail = &self->buffer[0];
    }
  }

  return length - write_len;
}

/* 借出当前可连续发送的数据，不推进 head。
 * 返回该节点当前已用总长度；*data / *length 为该连续块的起址与长度。
 * 如果节点为空，*data = NULL、*length = 0，返回 0。 */
static size_t EM_USART_Ex_TxBufferNode_PeekData(
    EM_USART_Ex_TxBufferNode_t *self, uint8_t **data, size_t *length) {
  if (data == NULL || length == NULL) {
    return 0;
  }
  *data = NULL;
  *length = 0;

  if (self == NULL) {
    return 0;
  }

  size_t used_size = EM_USART_Ex_TxBufferNode_GetUsedSize(self);
  if (used_size == 0) {
    return 0;
  }

  size_t chunk_len;
  if (self->buffer_head < self->buffer_tail) {
    chunk_len = (size_t)(self->buffer_tail - self->buffer_head);
  } else {
    chunk_len =
        (size_t)USART_BLOCK_SIZE - (size_t)(self->buffer_head - self->buffer);
  }

  *data = self->buffer_head;
  *length = chunk_len;
  return used_size;
}

/* DMA 已确认发送完成后，消费掉 length 字节，推进 head。
 * 返回消费后该节点剩余的数据长度。 */
static size_t EM_USART_Ex_TxBufferNode_PopData(EM_USART_Ex_TxBufferNode_t *self,
                                               size_t length) {
  if (self == NULL) {
    return 0;
  }

  size_t used_size = EM_USART_Ex_TxBufferNode_GetUsedSize(self);
  if (length > used_size) {
    length = used_size; /* 防御：不能消费超过已有数据 */
  }

  self->buffer_head += length;
  if (self->buffer_head >= &self->buffer[USART_BLOCK_SIZE]) {
    self->buffer_head -= USART_BLOCK_SIZE;
  }

  return EM_USART_Ex_TxBufferNode_GetUsedSize(self);
}

/* ============================ Handler 链 ============================ */

typedef struct EM_USART_Ex_RxHandlerNode_t {
  struct EM_USART_Ex_RxHandlerNode_t *next;
  EM_USART_Ex_RxHandler handler;
} EM_USART_Ex_RxHandlerNode_t;

typedef struct EM_USART_Ex_TxHandlerNode_t {
  struct EM_USART_Ex_TxHandlerNode_t *next;
  EM_USART_Ex_TxHandler handler;
} EM_USART_Ex_TxHandlerNode_t;

/* ============================ Ex 实例 ============================ */

typedef struct EM_USART_Ex_t {
  EM_USART_Ex_TxBufferNode_t *tx_chain;
  EM_USART_Ex_RxHandlerNode_t *rx_handler_chain;
  EM_USART_Ex_TxHandlerNode_t *tx_handler_chain;
  uint8_t *rx_blocks[USART_EX_RX_BLOCKS];
  uint8_t rx_arm_index;
  volatile uint8_t is_busy;
  volatile uint8_t is_inited;
  size_t current_dma_sending_len; /* 本次 DMA 正在发送的字节数 */
  uint8_t tx_block_count;         /* 当前 TX 链占用块数（上限 USART_TX_MAX_BLOCKS） */
  uint8_t warn_count;             /* 当前待发的 WARN 块数（上限 USART_TX_MAX_WARN_BLOCKS） */
} EM_USART_Ex_t;

/* 静态全局变量：按 EUUEN 枚举直接索引 */
static EM_USART_Ex_t s_usart_ex[EUUEN_USART_NUM];

/* 全部串口 RX 块共享的全局计数（上限 USART_RX_MAX_BLOCKS） */
static uint8_t s_usart_ex_rx_blocks_used = 0;

static EM_USART_Ex_t *EM_USART_Ex_Get(Euuen_USART_ID usart) {
  if (usart < 0 || usart >= EUUEN_USART_NUM) {
    return NULL;
  }
  if (!s_usart_ex[usart].is_inited) {
    return NULL;
  }
  return &s_usart_ex[usart];
}

/* ============================ 发送泵 ============================ */

/* 任务上下文使用：内部自己获取任务级临界区。忙则直接返回。 */
static void EM_USART_Ex_StartSendData(Euuen_USART_ID usart) {
  EM_USART_Ex_t *self = &s_usart_ex[usart];

  EM_EnterCritical();

  if (self->is_busy) {
    EM_ExitCritical();
    return; /* 正在发，不重复启动 */
  }

  EM_USART_Ex_TxBufferNode_t *head = self->tx_chain;
  uint8_t *data = NULL;
  size_t length = 0;
  EM_USART_Ex_TxBufferNode_PeekData(head, &data, &length);

  if (length > 0) {
    self->is_busy = 1;
    self->current_dma_sending_len = length;
    EM_USART_SendData_DMA(usart, data, length);
  } else {
    self->is_busy = 0;
    self->current_dma_sending_len = 0;
  }

  EM_ExitCritical();
}

/* 驱动 TX 完成入口（ISR 上下文）：
 * 消费已发数据 → 释放空节点 → 续发下一段 → 队空时通知 TX handler 链。 */
static void EM_USART_Ex_OnTxComplete(Euuen_USART_ID usart) {
  if (usart < 0 || usart >= EUUEN_USART_NUM) {
    return;
  }
  EM_USART_Ex_t *self = &s_usart_ex[usart];
  if (!self->is_inited) {
    return;
  }

  uint32_t critical_handle = EM_EnterCriticalFromISR();

  EM_USART_Ex_TxBufferNode_t *head = self->tx_chain;

  /* 1. 消费掉刚才 DMA 发完的那段，head 推进 */
  EM_USART_Ex_TxBufferNode_PopData(head, self->current_dma_sending_len);
  self->current_dma_sending_len = 0;

  /* 2. 头节点空了：摘链并释放；WARN 特殊块不计入 tx_block_count */
  if (EM_USART_Ex_TxBufferNode_GetUsedSize(head) == 0) {
    if (head->is_warn && self->warn_count > 0) {
      self->warn_count--; /* 该告警已发完 */
    }
    if (head->next != NULL) {
      self->tx_chain = head->next;
      if (!head->is_warn && self->tx_block_count > 0) {
        self->tx_block_count--;
      }
      MemAllocator_FreeFromISR(&ram_allocator, head);
      head = self->tx_chain;
    }
  }

  /* 3. 取下一段数据，启动 DMA；没有则置空闲 */
  uint8_t *data = NULL;
  size_t length = 0;
  EM_USART_Ex_TxBufferNode_PeekData(head, &data, &length);

  if (length > 0) {
    self->is_busy = 1;
    self->current_dma_sending_len = length;
    EM_USART_SendData_DMA(usart, data, length);
  } else {
    self->is_busy = 0;
    self->current_dma_sending_len = 0;
  }

  EM_ExitCriticalFromISR(critical_handle);

  /* 4. 队空 → 通知 TX handler 链（ISR 上下文，回调需保持短小） */
  if (self->is_busy == 0) {
    for (EM_USART_Ex_TxHandlerNode_t *cur = self->tx_handler_chain;
         cur != NULL; cur = cur->next) {
      if (cur->handler != NULL) {
        cur->handler();
      }
    }
  }
}

/* ============================ 接收泵 ============================ */

static void EM_USART_Ex_ArmRx(Euuen_USART_ID usart) {
  EM_USART_Ex_t *self = &s_usart_ex[usart];
  uint8_t *blk = self->rx_blocks[self->rx_arm_index];
  if (blk == NULL) {
    return;
  }
  EM_USART_EnableRxHandler(usart, blk, USART_BLOCK_SIZE);
}

/* 驱动 RX 事件入口（ISR 上下文）：分发 → 轮转下一块 → 重新 arm。 */
static void EM_USART_Ex_OnRxEvent(Euuen_USART_ID usart, uint8_t *data,
                                  size_t size) {
  if (usart < 0 || usart >= EUUEN_USART_NUM) {
    return;
  }
  EM_USART_Ex_t *self = &s_usart_ex[usart];
  if (!self->is_inited) {
    return;
  }

  if (data != NULL && size > 0) {
    for (EM_USART_Ex_RxHandlerNode_t *cur = self->rx_handler_chain; cur != NULL;
         cur = cur->next) {
      if (cur->handler != NULL) {
        cur->handler(data, (uint16_t)size);
      }
    }
  }

  /* 轮转到下一块缓冲，重新启动一次接收 */
  self->rx_arm_index =
      (uint8_t)((self->rx_arm_index + 1) % USART_EX_RX_BLOCKS);
  EM_USART_Ex_ArmRx(usart);
}

/* ============================ 公共 API ============================ */

void EM_USART_Ex_Init(Euuen_USART_ID usart) {
  if (usart < 0 || usart >= EUUEN_USART_NUM) {
    return;
  }

  EM_USART_Ex_t *self = &s_usart_ex[usart];

  /* 防重复初始化：资源分配失败时不置 is_inited，允许后续重试 */
  if (self->is_inited) {
    LOG_WARN("[EM_USART_Ex_Init] usart %d already initialized, skipped",
             (int)usart);
    return;
  }

  EM_USART_Init(usart); /* interface 驱动初始化（自身也带防重复） */
  EM_USART_SetTxHandler(usart, EM_USART_Ex_OnTxComplete);

  self->tx_chain =
      MemAllocator_Alloc(&ram_allocator, sizeof(EM_USART_Ex_TxBufferNode_t));
  if (self->tx_chain == NULL) {
    LOG_ERROR("[EM_USART_Ex_Init] usart %d tx buffer alloc failed",
              (int)usart);
    return;
  }
  EM_USART_Ex_TxBufferNode_Init(self->tx_chain);
  self->tx_block_count = 1;
  self->warn_count = 0;

  self->rx_handler_chain = NULL;
  self->tx_handler_chain = NULL;
  for (uint8_t i = 0; i < USART_EX_RX_BLOCKS; i++) {
    self->rx_blocks[i] = NULL;
  }
  self->rx_arm_index = 0;
  self->is_busy = 0;
  self->current_dma_sending_len = 0;
  self->is_inited = 1;
}

void EM_USART_Ex_EnableRx(Euuen_USART_ID usart) {
  EM_USART_Ex_t *self = EM_USART_Ex_Get(usart);
  if (self == NULL) {
    return;
  }

  /* 分配 RX 块池（已分配过则跳过，天然防重复），受全局块数上限约束 */
  for (uint8_t i = 0; i < USART_EX_RX_BLOCKS; i++) {
    if (self->rx_blocks[i] == NULL) {
      if (s_usart_ex_rx_blocks_used >= USART_RX_MAX_BLOCKS) {
        LOG_ERROR("[EM_USART_Ex_EnableRx] usart %d rx block pool exhausted",
                  (int)usart);
        return;
      }
      self->rx_blocks[i] = MemAllocator_Alloc(&ram_allocator, USART_BLOCK_SIZE);
      if (self->rx_blocks[i] == NULL) {
        LOG_ERROR("[EM_USART_Ex_EnableRx] usart %d rx block alloc failed",
                  (int)usart);
        return;
      }
      s_usart_ex_rx_blocks_used++;
    }
  }

  self->rx_arm_index = 0;
  EM_USART_SetRxHandler(usart, EM_USART_Ex_OnRxEvent);
  EM_USART_Ex_ArmRx(usart);
}

void EM_USART_Ex_AddRxHandler(Euuen_USART_ID usart,
                              EM_USART_Ex_RxHandler handler) {
  if (handler == NULL) {
    return;
  }

  EM_USART_Ex_t *self = EM_USART_Ex_Get(usart);
  if (self == NULL) {
    return;
  }

  EM_EnterCritical();

  /* 遍历，顺便查重、找尾 */
  EM_USART_Ex_RxHandlerNode_t *cur = self->rx_handler_chain;
  EM_USART_Ex_RxHandlerNode_t *tail = NULL;
  while (cur != NULL) {
    if (cur->handler == handler) {
      EM_ExitCritical();
      return; /* 已注册，不重复添加 */
    }
    tail = cur;
    cur = cur->next;
  }

  EM_USART_Ex_RxHandlerNode_t *node =
      MemAllocator_Alloc(&ram_allocator, sizeof(EM_USART_Ex_RxHandlerNode_t));
  if (node == NULL) {
    EM_ExitCritical();
    return; /* 分配失败，静默返回 */
  }
  node->handler = handler;
  node->next = NULL;

  if (tail == NULL) {
    self->rx_handler_chain = node; /* 空链表 */
  } else {
    tail->next = node; /* 追加到尾部 */
  }

  EM_ExitCritical();
}

void EM_USART_Ex_AddTxHandler(Euuen_USART_ID usart,
                              EM_USART_Ex_TxHandler handler) {
  if (handler == NULL) {
    return;
  }

  EM_USART_Ex_t *self = EM_USART_Ex_Get(usart);
  if (self == NULL) {
    return;
  }

  EM_EnterCritical();

  /* 遍历，顺便查重、找尾 */
  EM_USART_Ex_TxHandlerNode_t *cur = self->tx_handler_chain;
  EM_USART_Ex_TxHandlerNode_t *tail = NULL;
  while (cur != NULL) {
    if (cur->handler == handler) {
      EM_ExitCritical();
      return; /* 已注册，不重复添加 */
    }
    tail = cur;
    cur = cur->next;
  }

  EM_USART_Ex_TxHandlerNode_t *node =
      MemAllocator_Alloc(&ram_allocator, sizeof(EM_USART_Ex_TxHandlerNode_t));
  if (node == NULL) {
    EM_ExitCritical();
    return; /* 分配失败，静默返回 */
  }
  node->handler = handler;
  node->next = NULL;

  if (tail == NULL) {
    self->tx_handler_chain = node; /* 空链表 */
  } else {
    tail->next = node; /* 追加到尾部 */
  }

  EM_ExitCritical();
}

void EM_USART_Ex_SendStr(Euuen_USART_ID usart, const char *str) {
  EM_USART_Ex_SendData(usart, (const uint8_t *)str, strlen(str));
}

size_t EM_USART_Ex_SendData(Euuen_USART_ID usart, const uint8_t *data,
                            size_t length) {
  EM_USART_Ex_t *self = EM_USART_Ex_Get(usart);
  if (self == NULL || data == NULL || length == 0) {
    return 0;
  }

  EM_EnterCritical();

  /* 找链尾 */
  EM_USART_Ex_TxBufferNode_t *tail = self->tx_chain;
  while (tail->next != NULL) {
    tail = tail->next;
  }

  /* 预计算可用空间：尾节点剩余 + 还能新建的节点数 × 每节点容量。
   * WARN 特殊块不提供数据容量（否则等于给了一条超限的“后门”） */
  size_t free_size =
      tail->is_warn ? 0 : EM_USART_Ex_TxBufferNode_GetFreeSize(tail);
  uint8_t extra = (self->tx_block_count < USART_TX_MAX_BLOCKS)
                      ? (uint8_t)(USART_TX_MAX_BLOCKS - self->tx_block_count)
                      : 0;
  free_size += (size_t)extra * (USART_BLOCK_SIZE - 1U);

  if (length > free_size) {
    /* 整帧装不下 -> 完全拒绝，避免只塞一部分导致帧不完整；
     * 每次拒绝都现场新建一个 WARN 特殊块（不计入 tx_block_count），
     * 但待发告警块数受 USART_TX_MAX_WARN_BLOCKS 限制，防止刷屏吃光堆 */
    if (self->warn_count < USART_TX_MAX_WARN_BLOCKS) {
      char warn[48];
      int n = snprintf(warn, sizeof(warn),
                       "[WARN][usart_ex.c] tx full, drop %u\n",
                       (unsigned)length);
      if (n > 0) {
        if (n > (int)sizeof(warn) - 1) {
          n = (int)sizeof(warn) - 1;
        }
        EM_USART_Ex_TxBufferNode_t *warn_node = MemAllocator_Alloc(
            &ram_allocator, sizeof(EM_USART_Ex_TxBufferNode_t));
        if (warn_node != NULL) {
          EM_USART_Ex_TxBufferNode_Init(warn_node);
          warn_node->is_warn = 1;
          EM_USART_Ex_TxBufferNode_PutData(warn_node, (const uint8_t *)warn,
                                           (size_t)n);
          tail->next = warn_node;
          warn_node->next = NULL;
          self->warn_count++;
        }
      }
    }
    EM_ExitCritical();
    EM_USART_Ex_StartSendData(usart);
    return 0;
  }

  /* 预检通过：一定装得下，完整写入 */
  EM_USART_Ex_TxBufferNode_t *cur = tail;
  if (cur->is_warn) {
    /* WARN 块不用于普通数据：先新建一个普通节点作为写入起点 */
    EM_USART_Ex_TxBufferNode_t *node = MemAllocator_Alloc(
        &ram_allocator, sizeof(EM_USART_Ex_TxBufferNode_t));
    if (node == NULL) {
      /* block 耗尽 */
      for (;;) {
      }
    }
    EM_USART_Ex_TxBufferNode_Init(node);
    cur->next = node;
    cur = node;
    self->tx_block_count++;
  }

  size_t queued = 0;
  while (queued < length) {
    size_t to_write = length - queued;
    size_t remain =
        EM_USART_Ex_TxBufferNode_PutData(cur, data + queued, to_write);
    queued += to_write - remain;

    if (remain == 0) {
      break; /* 全部塞进去了 */
    }

    EM_USART_Ex_TxBufferNode_t *new_node = MemAllocator_Alloc(
        &ram_allocator, sizeof(EM_USART_Ex_TxBufferNode_t));
    if (new_node == NULL) {
      /* block 耗尽 */
      for (;;) {
      }
    }
    EM_USART_Ex_TxBufferNode_Init(new_node);
    cur->next = new_node;
    cur = new_node;
    self->tx_block_count++;
  }

  EM_ExitCritical();

  /* 入队完成后再启动；StartSendData 内部自会判 is_busy，忙则自动跳过 */
  if (queued > 0) {
    EM_USART_Ex_StartSendData(usart);
  }

  return queued;
}

uint8_t EM_USART_Ex_IsBusy(Euuen_USART_ID usart) {
  EM_USART_Ex_t *em_usart = EM_USART_Ex_Get(usart);
  if (em_usart == NULL) {
    return 0;
  }
  return (em_usart->is_busy != 0);
}
