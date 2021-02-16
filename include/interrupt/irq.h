#pragma once

#include <stdint.h>

namespace Interrupt {
    typedef void (*IRQHandler)();

    // Returns false if the irq is already taken
    bool InstallIRQHandler(uint8_t irq, IRQHandler handler, void* context);

    // Returns the context for the irq
    // Returns nullptr if no irq is installed
    void* RemoveIRQHandler(uint8_t irq);
} // namespace Interrupt