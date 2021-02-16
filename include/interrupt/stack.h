#pragma once

#include <stdint.h>

namespace Interrupt {
    void SetInterruptStack(uint64_t stackBase);
}