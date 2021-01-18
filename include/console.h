#pragma once

#include <types.h>

namespace Console {
#pragma pack(push)
#pragma pack(1)

    struct GraphicsInfo {
        uint32_t horizontalResolution;
        uint32_t verticalResolution;
        uint32_t pixelFormat;
        uint32_t redMask;
        uint32_t greenMask;
        uint32_t blueMask;
        uint32_t reserved;
        uint32_t pixelsPerScanline;
        uint64_t frameBufferBase;
        uint64_t frameBufferSize;
    };

#pragma pack(pop)

    void Init(GraphicsInfo* gmode);

    bool DisplayCharacter(char character);
    int DisplayString(const char* string);

    void SetForegroundColor(uint32_t color);
    void SetBackgroundColor(uint32_t color);
    void ClearScreen();
} // namespace Console