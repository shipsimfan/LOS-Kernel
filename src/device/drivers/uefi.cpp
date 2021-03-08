#include <device/drivers/uefi.h>

#include <bootloader.h>
#include <console.h>
#include <device/manager.h>
#include <errno.h>

extern uint8_t font[];

UEFIVideoDevice::UEFIVideoDevice() : Device("UEFI GOP Video", Type::CONSOLE), foregroundColor(0xFFFFFFFF), backgroundColor(0) {
    framebuffer = (uint32_t*)((uint64_t)gopInfo->frameBufferBase + 0xFFFF800000000000);
    backbuffer = new uint32_t[gopInfo->frameBufferSize];
    framebufferSize = gopInfo->frameBufferSize;
    pixelsPerScanline = gopInfo->pixelsPerScanline;
    width = gopInfo->horizontalResolution;
    height = gopInfo->verticalResolution;

    consoleWidth = width / 8;
    consoleHeight = height / 16;

    ClearScreen();
}

UEFIVideoDevice::~UEFIVideoDevice() { delete backbuffer; }

uint64_t UEFIVideoDevice::Read(uint64_t address, uint64_t* value) {
    switch (address) {
    case CURSOR_X_ADDRESS:
        *value = cursorX;
        break;

    case CURSOR_Y_ADDRESS:
        *value = cursorY;
        break;

    case FOREGROUND_COLOR_ADDRESS:
        *value = foregroundColor;
        break;

    case BACKGROUND_COLOR_ADDRESS:
        *value = backgroundColor;
        break;

    default:
        return ERROR_NOT_IMPLEMENTED;
    }

    return SUCCESS;
}

uint64_t UEFIVideoDevice::Write(uint64_t address, uint64_t value) {
    switch (address) {
    case CURSOR_X_ADDRESS:
        if (value > consoleWidth)
            return ERROR_OUT_OF_RANGE;
        cursorX = value;
        break;

    case CURSOR_Y_ADDRESS:
        if (value > consoleHeight)
            return ERROR_OUT_OF_RANGE;
        cursorY = value;
        break;

    case FOREGROUND_COLOR_ADDRESS:
        foregroundColor = value;
        break;

    case BACKGROUND_COLOR_ADDRESS:
        backgroundColor = value;
        break;

    case CLEAR_SCREEN_ADDRESS:
        ClearScreen();
        break;

    default:
        return ERROR_NOT_IMPLEMENTED;
    }

    return SUCCESS;
}

int64_t UEFIVideoDevice::WriteStream(uint64_t address, void* buffer, int64_t count) {
    int64_t countWritten = 0;
    char* string = (char*)buffer;
    for (int i = 0; string[i] && i < count; i++)
        if (DisplayCharacter(string[i]))
            countWritten++;

    return countWritten;
}

void UEFIVideoDevice::PlotPixel(uint32_t x, uint32_t y, uint32_t pixel) {
    if (x >= width || y >= height)
        return;

    framebuffer[x + y * pixelsPerScanline] = pixel;
    backbuffer[x + y * pixelsPerScanline] = pixel;
}

void UEFIVideoDevice::RenderCharacter(char character, uint32_t x, uint32_t y) {
    int mask[8] = {1, 2, 4, 8, 16, 32, 64, 128};
    uint8_t* glyph = (uint8_t*)((uint64_t)font + (uint64_t)character * 16);

    for (uint32_t cy = 0; cy < 16; cy++)
        for (uint32_t cx = 0; cx < 8; cx++)
            PlotPixel(x + 8 - cx, y + cy - 12, glyph[cy] & mask[cx] ? foregroundColor : backgroundColor);
}

bool UEFIVideoDevice::DisplayCharacter(char character) {
    bool ret = false;

    switch (character) {
    case '\n':
        for (; cursorX < consoleWidth; cursorX++)
            RenderCharacter(' ', cursorX * 8, cursorY * 16);

        break;

    default:
        RenderCharacter(character, cursorX * 8, cursorY * 16);
        cursorX++;
        ret = true;
        break;
    }

    if (cursorX >= consoleWidth) {
        cursorX = 0;
        cursorY++;
    }

    if (cursorY >= consoleHeight)
        ScrollUp();

    return ret;
}

void UEFIVideoDevice::ScrollUp() {
    uint64_t* fBuffer = (uint64_t*)framebuffer;
    uint64_t* bBuffer = (uint64_t*)backbuffer;
    uint64_t diff = 8 * pixelsPerScanline;

    for (uint64_t i = 0; i < (framebufferSize / 8) - diff; i++) {
        fBuffer[i] = bBuffer[i + diff];
        bBuffer[i] = bBuffer[i + diff];
    }

    for (uint64_t i = (framebufferSize / 8) - diff; i < (framebufferSize / 8); i++) {
        fBuffer[i] = 0;
        bBuffer[i] = 0;
    }

    cursorY--;
}

void UEFIVideoDevice::ClearScreen() {
    for (uint64_t i = 0; i < framebufferSize; i++)
        framebuffer[i] = backgroundColor;

    cursorX = 0;
    cursorY = 1;
}

void InitializeUEFIVideoDriver() {
    UEFIVideoDevice* videoDevice = new UEFIVideoDevice;
    Device::RegisterDevice(nullptr, videoDevice);
    Console::SetVideoDevice(videoDevice);
}