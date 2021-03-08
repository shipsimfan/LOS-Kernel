#pragma once

#include <device/device.h>
#include <stdint.h>

#define CURSOR_X_ADDRESS 0x00
#define CURSOR_Y_ADDRESS 0x01
#define FOREGROUND_COLOR_ADDRESS 0x02
#define BACKGROUND_COLOR_ADDRESS 0x03
#define CLEAR_SCREEN_ADDRESS 0x05

namespace Console {
    void SetStdOutput(Device::Device* device);
    void SetStdInput(Device::Device* device);

    int Print(const char* format, ...);
    int Println(const char* format, ...);

    int64_t Read(void* buffer, int64_t count);

    void SetForegroundColor(uint8_t red, uint8_t green, uint8_t blue);
    void SetBackgroundColor(uint8_t red, uint8_t green, uint8_t blue);

    void SetCursorPos(uint32_t x, uint32_t y);
    uint32_t GetCursorX();
    uint32_t GetCursorY();

    void ClearScreen();
} // namespace Console