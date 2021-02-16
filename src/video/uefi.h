#pragma once

#include <stdint.h>

#pragma pack(push)
#pragma pack(1)

struct GOPInfo {
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