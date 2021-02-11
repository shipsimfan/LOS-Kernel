#include <console.h>

#include <font.h>
#include <proc.h>
#include <stdlib.h>

namespace Console {
    uint32_t foregroundColor, backgroundColor;
    uint32_t* framebuffer;
    uint32_t pixelsPerScanline;
    uint64_t framebufferSize;

    uint32_t width, height;

    uint32_t consoleWidth, consoleHeight;
    uint32_t consoleX = 0, consoleY = 1;

    bool isDoubleBuffer;
    uint32_t* backBuffer;

    Mutex videoMutex;

    void PlotPixel(uint32_t x, uint32_t y, uint32_t pixel) {
        if (x >= width || y >= height)
            return;

        framebuffer[x + y * pixelsPerScanline] = pixel;
        if (isDoubleBuffer)
            backBuffer[x + y * pixelsPerScanline] = pixel;
    }

    void Init(GraphicsInfo* gmode) {
        isDoubleBuffer = false;

        foregroundColor = 0xFFFFFFFF;
        backgroundColor = 0x00000000;

        framebuffer = (uint32_t*)((uint64_t)gmode->frameBufferBase + 0xFFFF800000000000);
        pixelsPerScanline = gmode->pixelsPerScanline;
        width = gmode->horizontalResolution;
        height = gmode->verticalResolution;
        framebufferSize = gmode->frameBufferSize;

        consoleWidth = width / 8;
        consoleHeight = height / 16;

        videoMutex.queue = nullptr;
        videoMutex.value = true;

        ClearScreen();
    }

    void InitDoubleBuffering() {
        backBuffer = (uint32_t*)malloc(framebufferSize);
        if (backBuffer == nullptr)
            return;

        uint64_t* fBuffer = (uint64_t*)framebuffer;
        uint64_t* bBuffer = (uint64_t*)backBuffer;

        for (uint64_t i = 0; i < framebufferSize / 8; i++)
            bBuffer[i] = fBuffer[i];

        isDoubleBuffer = true;
    }

    void RenderCharacter(char character, uint32_t x, uint32_t y) {
        int mask[8] = {1, 2, 4, 8, 16, 32, 64, 128};
        uint8_t* glyph = (uint8_t*)((uint64_t)font + (uint64_t)character * 16);

        for (uint32_t cy = 0; cy < 16; cy++)
            for (uint32_t cx = 0; cx < 8; cx++)
                PlotPixel(x + 8 - cx, y + cy - 12, glyph[cy] & mask[cx] ? foregroundColor : backgroundColor);
    }

    void ScrollUp() {
        uint64_t* fBuffer = (uint64_t*)framebuffer;
        uint64_t* bBuffer = (uint64_t*)backBuffer;
        uint64_t diff = 8 * pixelsPerScanline;

        for (uint64_t i = 0; i < (framebufferSize / 8) - diff; i++) {
            fBuffer[i] = bBuffer[i + diff];
            bBuffer[i] = bBuffer[i + diff];
        }

        for (uint64_t i = (framebufferSize / 8) - diff; i < (framebufferSize / 8); i++) {
            fBuffer[i] = 0;
            bBuffer[i] = 0;
        }

        consoleY--;
    }

    bool DisplayCharacter(char character) {
        videoMutex.Lock();
        bool ret = false;

        switch (character) {
        case '\n':
            for (; consoleX < consoleWidth; consoleX++)
                RenderCharacter(' ', consoleX * 8, consoleY * 16);

            break;

        default:
            RenderCharacter(character, consoleX * 8, consoleY * 16);
            consoleX++;
            ret = true;
            break;
        }

        if (consoleX >= consoleWidth) {
            consoleX = 0;
            consoleY++;
        }

        if (consoleY >= consoleHeight) {
            if (isDoubleBuffer)
                ScrollUp();
            else {
                consoleX = 0;
                consoleY = 1;
            }
        }

        videoMutex.Unlock();
        return ret;
    }

    int DisplayString(const char* string) {
        int count = 0;
        for (int i = 0; string[i]; i++)
            if (DisplayCharacter(string[i]))
                count++;

        return count;
    }

    void SetForegroundColor(uint32_t color) { foregroundColor = color; }
    void SetBackgroundColor(uint32_t color) { backgroundColor = color; }

    void ClearScreen() {
        for (uint64_t i = 0; i < framebufferSize / 4; i++) {
            framebuffer[i] = backgroundColor;
            if (isDoubleBuffer)
                backBuffer[i] = backgroundColor;
        }

        consoleX = 0;
        consoleY = 1;
    }

    uint64_t GetFramebuffer() { return (uint64_t)framebuffer; }

    uint64_t GetFramebufferSize() { return framebufferSize; }
} // namespace Console