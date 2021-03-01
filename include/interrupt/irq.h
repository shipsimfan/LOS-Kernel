#pragma once

#include <stdint.h>

namespace Interrupt {
    typedef void (*IRQHandler)(void* context);

    // Returns false if the irq is already taken
    bool InstallIRQHandler(uint8_t irq, IRQHandler handler, void* context);

    // Returns false if the irq hasn't been installed yet
    bool SetIRQContext(uint8_t irq, void* newContext);

    // Returns the context for the irq
    // Returns nullptr if no irq is installed
    void* RemoveIRQHandler(uint8_t irq);
} // namespace Interrupt