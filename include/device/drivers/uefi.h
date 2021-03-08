#pragma once

#include <device/device.h>

class UEFIVideoDevice : public Device::Device {
public:
    UEFIVideoDevice();
    ~UEFIVideoDevice();

protected:
    uint64_t OnOpen() override;
    uint64_t OnClose() override;

    uint64_t DoRead(uint64_t address, uint64_t* value) override;
    uint64_t DoReadStream(uint64_t address, void* buffer, int64_t count, int64_t& countRead) override;

    uint64_t DoWrite(uint64_t address, uint64_t value) override;
    uint64_t DoWriteStream(uint64_t address, void* buffer, int64_t count, int64_t& countWritten) override;

private:
    void PlotPixel(uint32_t x, uint32_t y, uint32_t pixel);
    void RenderCharacter(char character, uint32_t x, uint32_t y);
    bool DisplayCharacter(char character);
    void ScrollUp();
    void ClearScreen();

    uint32_t cursorX, cursorY;
    uint32_t consoleWidth, consoleHeight;

    uint32_t foregroundColor, backgroundColor;

    Mutex framebufferMutex;
    uint32_t* framebuffer;
    uint32_t* backbuffer;
    uint64_t framebufferSize;
    uint32_t pixelsPerScanline;
    uint32_t width;
    uint32_t height;
};

void InitializeUEFIVideoDriver();