#pragma once

#ifdef __cplusplus
 extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

/* 对齐要求 */
#define MEM_ALIGNMENT 8
#define MEM_ALIGN_MASK (MEM_ALIGNMENT - 1)

typedef enum {
  MEM_POLICY_NEW_FREE_WITH_MERGE = 0,
} MemPolicy;

typedef struct MemBlockLink {
  struct MemBlockLink *pxNextFreeBlock;
  size_t xBlockSize;
} MemBlockLink_t;

#define MEM_BLOCK_LINK_SIZE                                                    \
  ((sizeof(MemBlockLink_t) + MEM_ALIGN_MASK) & ~(size_t)MEM_ALIGN_MASK)

typedef struct {
  MemBlockLink_t xStart;
  MemBlockLink_t *pxEnd;
  size_t xFreeBytesRemaining;
  size_t xMinimumEverFreeBytesRemaining;
  size_t xNumberOfSuccessfulAllocations;
  size_t xNumberOfSuccessfulFrees;
  size_t xBlockAllocatedBit;
  MemPolicy policy;
  void *buffer;
  size_t bufferSize;
} MemAllocator;

/* 构造：非线程安全，仅在单任务/初始化阶段调用 */
void MemAllocator_Init(MemAllocator *self, void *buffer, size_t size,
                       MemPolicy policy);

/* 任务上下文：内部使用 BSP_taskENTER/EXIT_CRITICAL */
void *MemAllocator_Alloc(MemAllocator *self, size_t wantedSize);
void MemAllocator_Free(MemAllocator *self, void *ptr);

/* 中断上下文：内部使用 BSP_taskENTER/EXIT_CRITICAL_FROM_ISR */
void *MemAllocator_AllocFromISR(MemAllocator *self, size_t wantedSize);
void MemAllocator_FreeFromISR(MemAllocator *self, void *ptr);

/* 查询 */
size_t MemAllocator_GetFreeSize(MemAllocator *self);
size_t MemAllocator_GetMinEverFreeSize(MemAllocator *self);

#ifdef __cplusplus
}
#endif
