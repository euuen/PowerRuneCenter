#include "euuen_mod.h"


uint8_t ram_allocator_buffer[MEM_ALLOCATOR_RAM_HEAP_SIZE]
    __attribute__((aligned(MEM_ALIGNMENT)));
MemAllocator ram_allocator;

// 如果你的单片机没有ccmram，可以把__attribute__((section(".ccmram")))去掉，这样也可以兼容使用本模块。
uint8_t ccmram_allocator_buffer[MEM_ALLOCATOR_CCMRAM_HEAP_SIZE]
    __attribute__((aligned(MEM_ALIGNMENT))) __attribute__((section(".ccmram")));
MemAllocator ccmram_allocator;

static uint8_t s_context_made = 0;

void EM_MakeContext(void) {
  if (s_context_made)
    return;
  s_context_made = 1;

  MemAllocator_Init(&ram_allocator, ram_allocator_buffer,
                    MEM_ALLOCATOR_RAM_HEAP_SIZE,
                    MEM_POLICY_NEW_FREE_WITH_MERGE);
  MemAllocator_Init(&ccmram_allocator, ccmram_allocator_buffer,
                    MEM_ALLOCATOR_CCMRAM_HEAP_SIZE,
                    MEM_POLICY_NEW_FREE_WITH_MERGE);
}