#pragma once

#include <stdint.h>

namespace Console {
    int Print(const char* format, ...);
    int Println(const char* format, ...);

    void SetForegroundColor(uint8_t red, uint8_t green, uint8_t blue);
    void SetBackgroundColor(uint8_t red, uint8_t green, uint8_t blue);

    void SetCursorPos(uint32_t x, uint32_t y);
    uint32_t GetCursorX();
    uint32_t GetCursorY();

    void ScrollUp();
    void ClearScreen();
} // namespace Console