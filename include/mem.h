#pragma once

#include <multiboot2.h>

namespace MemoryManager {
    bool Init(multiboot2BootInformation* bootInfo);
}