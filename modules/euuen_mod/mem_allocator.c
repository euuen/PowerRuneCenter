#include "mem_allocator.h"
#include <euuen_mod/euuen_mod.h>
#include <string.h>

/* ---------------------------------------------------------------------------
 * 内部实现（无锁）：把所有"非保护"逻辑集中在这里，
 * 由外层函数决定是用普通临界区还是 ISR 临界区来保护。
 * ------------------------------------------------------------------------ */

static void prvInsertBlockIntoFreeList(MemAllocator *self,
                                       MemBlockLink_t *pxBlockToInsert) {
  MemBlockLink_t *pxIterator;
  uint8_t *puc;

  for (pxIterator = &self->xStart;
       pxIterator->pxNextFreeBlock < pxBlockToInsert;
       pxIterator = pxIterator->pxNextFreeBlock) {
  }

  puc = (uint8_t *)pxIterator;
  if ((puc + pxIterator->xBlockSize) == (uint8_t *)pxBlockToInsert) {
    pxIterator->xBlockSize += pxBlockToInsert->xBlockSize;
    pxBlockToInsert = pxIterator;
  }

  puc = (uint8_t *)pxBlockToInsert;
  if ((puc + pxBlockToInsert->xBlockSize) ==
      (uint8_t *)pxIterator->pxNextFreeBlock) {
    if (pxIterator->pxNextFreeBlock != self->pxEnd) {
      pxBlockToInsert->xBlockSize += pxIterator->pxNextFreeBlock->xBlockSize;
      pxBlockToInsert->pxNextFreeBlock =
          pxIterator->pxNextFreeBlock->pxNextFreeBlock;
    } else {
      pxBlockToInsert->pxNextFreeBlock = self->pxEnd;
    }
  } else {
    pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;
  }

  if (pxIterator != pxBlockToInsert) {
    pxIterator->pxNextFreeBlock = pxBlockToInsert;
  }
}

/* 无锁分配核心 */
static void *prvAllocLocked(MemAllocator *self, size_t xWantedSize) {
  MemBlockLink_t *pxBlock, *pxPreviousBlock, *pxNewBlockLink;
  void *pvReturn = NULL;

  if (self == NULL || self->pxEnd == NULL)
    return NULL;

  if ((xWantedSize & self->xBlockAllocatedBit) == 0) {
    if (xWantedSize > 0) {
      xWantedSize += MEM_BLOCK_LINK_SIZE;
      if ((xWantedSize & MEM_ALIGN_MASK) != 0) {
        xWantedSize += (MEM_ALIGNMENT - (xWantedSize & MEM_ALIGN_MASK));
      }
    }

    if ((xWantedSize > 0) && (xWantedSize <= self->xFreeBytesRemaining)) {
      pxPreviousBlock = &self->xStart;
      pxBlock = self->xStart.pxNextFreeBlock;

      while ((pxBlock->xBlockSize < xWantedSize) &&
             (pxBlock->pxNextFreeBlock != NULL)) {
        pxPreviousBlock = pxBlock;
        pxBlock = pxBlock->pxNextFreeBlock;
      }

      if (pxBlock != self->pxEnd) {
        pvReturn = (void *)(((uint8_t *)pxPreviousBlock->pxNextFreeBlock) +
                            MEM_BLOCK_LINK_SIZE);
        pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

        if ((pxBlock->xBlockSize - xWantedSize) > (MEM_BLOCK_LINK_SIZE << 1)) {
          pxNewBlockLink = (void *)(((uint8_t *)pxBlock) + xWantedSize);
          pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - xWantedSize;
          pxBlock->xBlockSize = xWantedSize;
          prvInsertBlockIntoFreeList(self, pxNewBlockLink);
        }

        self->xFreeBytesRemaining -= pxBlock->xBlockSize;
        if (self->xFreeBytesRemaining < self->xMinimumEverFreeBytesRemaining) {
          self->xMinimumEverFreeBytesRemaining = self->xFreeBytesRemaining;
        }

        pxBlock->xBlockSize |= self->xBlockAllocatedBit;
        pxBlock->pxNextFreeBlock = NULL;
        self->xNumberOfSuccessfulAllocations++;
      }
    }
  }

  return pvReturn;
}

/* 无锁释放核心 */
static void prvFreeLocked(MemAllocator *self, void *pv) {
  uint8_t *puc = (uint8_t *)pv;
  MemBlockLink_t *pxLink;

  if (self == NULL || pv == NULL || self->pxEnd == NULL)
    return;

  puc -= MEM_BLOCK_LINK_SIZE;
  pxLink = (void *)puc;

  if ((pxLink->xBlockSize & self->xBlockAllocatedBit) != 0) {
    if (pxLink->pxNextFreeBlock == NULL) {
      pxLink->xBlockSize &= ~self->xBlockAllocatedBit;
      self->xFreeBytesRemaining += pxLink->xBlockSize;
      prvInsertBlockIntoFreeList(self, pxLink);
      self->xNumberOfSuccessfulFrees++;
    }
  }
}

/* ---------------------------------------------------------------------------
 * 构造（无需保护，建议初始化阶段调用）
 * ------------------------------------------------------------------------ */
void MemAllocator_Init(MemAllocator *self, void *buffer, size_t size,
                       MemPolicy policy) {
  MemBlockLink_t *pxFirstFreeBlock;
  uint8_t *pucAlignedHeap;
  size_t uxAddress;
  size_t xTotalHeapSize = size;

  if (self == NULL || buffer == NULL)
    return;

  memset(self, 0, sizeof(*self));
  self->policy = policy;
  self->buffer = buffer;
  self->bufferSize = size;

  if (policy != MEM_POLICY_NEW_FREE_WITH_MERGE)
    return;
  if (xTotalHeapSize < MEM_BLOCK_LINK_SIZE * 2)
    return;

  uxAddress = (size_t)buffer;
  if ((uxAddress & MEM_ALIGN_MASK) != 0) {
    uxAddress += MEM_ALIGNMENT - 1;
    uxAddress &= ~(size_t)MEM_ALIGN_MASK;
    xTotalHeapSize -= uxAddress - (size_t)buffer;
  }
  pucAlignedHeap = (uint8_t *)uxAddress;

  self->xStart.pxNextFreeBlock = (void *)pucAlignedHeap;
  self->xStart.xBlockSize = 0;

  uxAddress = ((size_t)pucAlignedHeap) + xTotalHeapSize;
  uxAddress -= MEM_BLOCK_LINK_SIZE;
  uxAddress &= ~(size_t)MEM_ALIGN_MASK;
  self->pxEnd = (void *)uxAddress;
  self->pxEnd->xBlockSize = 0;
  self->pxEnd->pxNextFreeBlock = NULL;

  pxFirstFreeBlock = (void *)pucAlignedHeap;
  pxFirstFreeBlock->xBlockSize = uxAddress - (size_t)pxFirstFreeBlock;
  pxFirstFreeBlock->pxNextFreeBlock = self->pxEnd;

  self->xMinimumEverFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;
  self->xFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;
  self->xBlockAllocatedBit = ((size_t)1) << ((sizeof(size_t) * 8) - 1);
}

// Out Of Memory
// 堆内存耗尽，请检查你的程序是否发生了内存泄漏，或者是堆内存分配过少
static void MemAllocator_ReportOOM(MemAllocator *self) {
#if MEM_ALLOCATOR_BLOCK_ON_OOM == 1
  (void)self;
  for (;;) {
  }
#endif
}

/* ---------------------------------------------------------------------------
 * 任务上下文 API：用 BSP_taskENTER/EXIT_CRITICAL 保护
 * ------------------------------------------------------------------------ */
void *MemAllocator_Alloc(MemAllocator *self, size_t wantedSize) {
  void *pv;

  EM_EnterCritical();
  pv = prvAllocLocked(self, wantedSize);
  EM_ExitCritical();

  if (pv == NULL) {
    MemAllocator_ReportOOM(self);
  }

  return pv;
}

void MemAllocator_Free(MemAllocator *self, void *ptr) {

  EM_EnterCritical();
  prvFreeLocked(self, ptr);
  EM_ExitCritical();
}

/* ---------------------------------------------------------------------------
 * ISR 上下文 API：用 EM_EnterCriticalFromISR / ExitCriticalFromISR 保护
 * ------------------------------------------------------------------------ */
void *MemAllocator_AllocFromISR(MemAllocator *self, size_t wantedSize) {
  uint32_t handle;
  void *pv;

  handle = EM_EnterCriticalFromISR();
  pv = prvAllocLocked(self, wantedSize);
  EM_ExitCriticalFromISR(handle);

  return pv;
}

void MemAllocator_FreeFromISR(MemAllocator *self, void *ptr) {
  uint32_t handle;

  handle = EM_EnterCriticalFromISR();
  prvFreeLocked(self, ptr);
  EM_ExitCriticalFromISR(handle);
}

/* ---------------------------------------------------------------------------
 * 查询（也加锁，避免读到中间状态）
 * ------------------------------------------------------------------------ */
size_t MemAllocator_GetFreeSize(MemAllocator *self) {
  size_t v;

  EM_EnterCritical();
  v = (self != NULL) ? self->xFreeBytesRemaining : 0;
  EM_ExitCritical();

  return v;
}

size_t MemAllocator_GetMinEverFreeSize(MemAllocator *self) {
  size_t v;

  EM_EnterCritical();
  v = (self != NULL) ? self->xMinimumEverFreeBytesRemaining : 0;
  EM_ExitCritical();

  return v;
}