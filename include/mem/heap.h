#pragma once

#include <mem/defs.h>
#include <types.h>

namespace MemoryManager {
#pragma pack(push)
#pragma pack(1)

    struct HeapLLBlockHeader {
        HeapLLBlockHeader* nextBlock;
        uint64_t numFreeBlocks;
    };

    struct HeapLLNode {
        HeapLLNode* next;
        uint64_t base;
        uint64_t size;
    };

    struct HeapLLBlock {
        HeapLLBlockHeader header;
        HeapLLNode nodes[(PAGE_SIZE - sizeof(HeapLLBlockHeader)) / sizeof(HeapLLNode)];
    };

#pragma pack(pop)

    namespace Heap {
        void Init();

        void* malloc(size_t size);
        void free(void* ptr);

        void* allocate_aligned(size_t alignment, size_t size);
    }; // namespace Heap
} // namespace MemoryManager