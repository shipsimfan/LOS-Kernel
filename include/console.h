#pragma once

#include <types.h>

namespace Console {
    void DisplayCharacter(char character);
    int DisplayString(const char* string);

    void SetForegroundColor(uint8_t color);
    void SetBackgroundColor(uint8_t color);
    void ClearScreen();
} // namespace Console