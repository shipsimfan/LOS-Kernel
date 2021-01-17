#pragma once

#include <mem/physical.h>

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

    class Heap {
    public:
        void Init(Physical* physicalMem);

        void* malloc(size_t size);
        void free(void* ptr);

    private:
        uint64_t top;

        // FREE BLOCKS
        HeapLLBlock* firstFreeBlock;
        HeapLLNode* freeHead;

        // USED BLOCKS
        HeapLLBlock* firstUsedBlock;
        HeapLLNode* usedHead;
    };
} // namespace MemoryManager