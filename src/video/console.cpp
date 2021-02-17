#include <console.h>

#include <bootloader.h>
#include <panic.h>
#include <stdarg.h>
#include <string.h>

namespace Console {
    uint32_t cursorX, cursorY;
    uint32_t consoleWidth, consoleHeight;

    uint32_t foregroundColor, backgroundColor;

    uint32_t* framebuffer;
    uint32_t* backbuffer;
    uint64_t framebufferSize;
    uint32_t pixelsPerScanline;
    uint32_t width;
    uint32_t height;

    bool doubleBufferInit;

    extern uint8_t font[];

    extern "C" void InitConsole() {
        doubleBufferInit = false;

        SetForegroundColor(0xFF, 0xFF, 0xFF);
        SetBackgroundColor(0x00, 0x00, 0x00);

        framebuffer = (uint32_t*)((uint64_t)gopInfo->frameBufferBase + 0xFFFF800000000000);
        backbuffer = nullptr;
        framebufferSize = gopInfo->frameBufferSize;
        pixelsPerScanline = gopInfo->pixelsPerScanline;
        width = gopInfo->horizontalResolution;
        height = gopInfo->verticalResolution;

        consoleWidth = width / 8;
        consoleHeight = height / 16;

        ClearScreen();
    }

    extern "C" void InitDoubleBuffering() {
        backbuffer = new uint32_t[framebufferSize];
        for (uint64_t i = 0; i < framebufferSize; i++)
            backbuffer[i] = framebuffer[i];

        doubleBufferInit = true;
    }

    void PlotPixel(uint32_t x, uint32_t y, uint32_t pixel) {
        if (x >= width || y >= height)
            return;

        framebuffer[x + y * pixelsPerScanline] = pixel;
        if (doubleBufferInit)
            backbuffer[x + y * pixelsPerScanline] = pixel;
    }

    void RenderCharacter(char character, uint32_t x, uint32_t y) {
        int mask[8] = {1, 2, 4, 8, 16, 32, 64, 128};
        uint8_t* glyph = (uint8_t*)((uint64_t)font + (uint64_t)character * 16);

        for (uint32_t cy = 0; cy < 16; cy++)
            for (uint32_t cx = 0; cx < 8; cx++)
                PlotPixel(x + 8 - cx, y + cy - 12, glyph[cy] & mask[cx] ? foregroundColor : backgroundColor);
    }

    bool DisplayCharacter(char character) {
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

    int DisplayString(const char* string) {
        int count = 0;
        for (int i = 0; string[i]; i++)
            if (DisplayCharacter(string[i]))
                count++;

        return count;
    }

    bool IsDigit(char c) { return c >= '0' && c <= '9'; }

    char* __uint_str(uintmax_t i, char b[], int base, bool plusSignIfNeeded, bool spaceSignIfNeeded, int paddingNo, bool justify, bool zeroPad) {
        const char* digit = "0123456789abcdef";
        if (base == 17) {
            base = 16;
            digit = "0123456789ABCDEF";
        }

        char* p = b;
        if (plusSignIfNeeded)
            *p++ = '+';
        else if (spaceSignIfNeeded)
            *p++ = ' ';

        uintmax_t shifter = i;
        do {
            ++p;
            shifter = shifter / base;
        } while (shifter);

        *p = '\0';
        do {
            *--p = digit[i % base];
            i = i / base;
        } while (i);

        int padding = paddingNo - (int)strlen(b);
        if (padding < 0)
            padding = 0;

        if (justify) {
            while (padding--) {
                if (zeroPad)
                    b[strlen(b)] = '0';
                else
                    b[strlen(b)] = ' ';
            }
        } else {
            char a[256] = {0};
            while (padding--) {
                if (zeroPad)
                    a[strlen(a)] = '0';
                else
                    a[strlen(a)] = ' ';
            }

            strcat(a, b);
            strcpy(b, a);
        }

        return b;
    }

    char* __int_str(intmax_t i, char b[], int base, bool plusSignIfNeeded, bool spaceSignIfNeeded, int paddingNo, bool justify, bool zeroPad) {
        const char* digit = "0123456789abcdef";
        if (base == 17) {
            base = 16;
            digit = "0123456789ABCDEF";
        }

        char* p = b;
        if (i < 0) {
            *p++ = '-';
            i *= -1;
        } else if (plusSignIfNeeded)
            *p++ = '+';
        else if (spaceSignIfNeeded)
            *p++ = ' ';

        intmax_t shifter = i;
        do {
            ++p;
            shifter = shifter / base;
        } while (shifter);

        *p = '\0';
        do {
            *--p = digit[i % base];
            i = i / base;
        } while (i);

        int padding = paddingNo - (int)strlen(b);
        if (padding < 0)
            padding = 0;

        if (justify) {
            while (padding--) {
                if (zeroPad)
                    b[strlen(b)] = '0';
                else
                    b[strlen(b)] = ' ';
            }
        } else {
            char a[256] = {0};
            while (padding--) {
                if (zeroPad)
                    a[strlen(a)] = '0';
                else
                    a[strlen(a)] = ' ';
            }

            strcat(a, b);
            strcpy(b, a);
        }

        return b;
    }

    int Printv(const char* format, va_list list) {
        int chars = 0;
        char intStrBuffer[256] = {0};

        for (int i = 0; format[i]; i++) {
            if (list != nullptr && format[i] == '%') {
                i++;

                bool leftJustify = false;
                bool zeroPad = false;
                bool spaceNoSign = false;
                bool altForm = false;
                bool plusSign = false;
                bool extBreak = false;
                while (!extBreak) {
                    switch (format[i]) {
                    case '-':
                        leftJustify = true;
                        i++;
                        break;

                    case '+':
                        plusSign = true;
                        i++;
                        break;

                    case '#':
                        altForm = true;
                        i++;
                        break;

                    case ' ':
                        spaceNoSign = true;
                        i++;
                        break;

                    case '0':
                        zeroPad = true;
                        i++;

                    default:
                        extBreak = true;
                        break;
                    }
                }

                int lengthSpec = 0;
                while (IsDigit(format[i])) {
                    lengthSpec *= 10;
                    lengthSpec += format[i] - 48;
                    i++;
                }

                if (format[i] == '*') {
                    lengthSpec = va_arg(list, int);
                    i++;
                }

                int precSpec = 0;
                if (format[i] == '.') {
                    i++;
                    while (IsDigit(format[i])) {
                        precSpec *= 10;
                        precSpec += format[i] - 48;
                        i++;
                    }

                    if (format[i] == '*') {
                        precSpec = va_arg(list, int);
                        i++;
                    }
                } else
                    precSpec = 6;

                char length = '\0';
                if (format[i] == 'h' || format[i] == 'l' || format[i] == 'j' || format[i] == 'z' || format[i] == 't' || format[i] == 'L') {
                    length = format[i];
                    i++;

                    if (format[i] == 'h')
                        length = 'H';
                    else if (format[i] == 'l') {
                        length = 'q';
                        i++;
                    }
                }

                char specifier = format[i];
                memset(intStrBuffer, 0, 256);

                int base = 10;
                if (specifier == 'o') {
                    base = 8;
                    specifier = 'u';
                    if (altForm) {
                        DisplayCharacter('0');
                        chars++;
                    }
                }

                if (specifier == 'p') {
                    base = 16;
                    length = 'z';
                    specifier = 'u';
                }

                switch (specifier) {
                case 'x':
                    base = 16;
                case 'X':
                    base = base == 10 ? 17 : base;
                    if (altForm)
                        chars += DisplayString("0x");

                case 'u': {
                    switch (length) {
                    case 0: {
                        unsigned int integer = va_arg(list, unsigned int);
                        __uint_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        chars += DisplayString(intStrBuffer);
                        break;
                    }
                    case 'H': {
                        unsigned char integer = va_arg(list, unsigned int);
                        __uint_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        chars += DisplayString(intStrBuffer);
                        break;
                    }
                    case 'h': {
                        unsigned short integer = va_arg(list, unsigned int);
                        __uint_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        chars += DisplayString(intStrBuffer);
                        break;
                    }
                    case 'l': {
                        unsigned long integer = va_arg(list, unsigned long);
                        __uint_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        chars += DisplayString(intStrBuffer);
                        break;
                    }
                    case 'q': {
                        unsigned long long integer = va_arg(list, unsigned long long);
                        __uint_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        chars += DisplayString(intStrBuffer);
                        break;
                    }
                    case 'j': {
                        uintmax_t integer = va_arg(list, uintmax_t);
                        __uint_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        chars += DisplayString(intStrBuffer);
                        break;
                    }
                    case 'z': {
                        uint64_t integer = va_arg(list, uint64_t);
                        __uint_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        chars += DisplayString(intStrBuffer);
                        break;
                    }
                    case 't': {
                        uint64_t integer = va_arg(list, uint64_t);
                        __uint_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        chars += DisplayString(intStrBuffer);
                        break;
                    }
                    default:
                        break;
                    }
                    break;
                }

                case 'd':
                case 'i': {
                    switch (length) {
                    case 0: {
                        int integer = va_arg(list, int);
                        __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        chars += DisplayString(intStrBuffer);
                        break;
                    }
                    case 'H': {
                        char integer = va_arg(list, int);
                        __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        chars += DisplayString(intStrBuffer);
                        break;
                    }
                    case 'h': {
                        short integer = va_arg(list, int);
                        __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        chars += DisplayString(intStrBuffer);
                        break;
                    }
                    case 'l': {
                        long integer = va_arg(list, long);
                        __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        chars += DisplayString(intStrBuffer);
                        break;
                    }
                    case 'q': {
                        long long integer = va_arg(list, long long);
                        __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        chars += DisplayString(intStrBuffer);
                        break;
                    }
                    case 'j': {
                        intmax_t integer = va_arg(list, intmax_t);
                        __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        chars += DisplayString(intStrBuffer);
                        break;
                    }
                    case 'z': {
                        int64_t integer = va_arg(list, int64_t);
                        __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        chars += DisplayString(intStrBuffer);
                        break;
                    }
                    case 't': {
                        int64_t integer = va_arg(list, int64_t);
                        __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        chars += DisplayString(intStrBuffer);
                        break;
                    }
                    default:
                        break;
                    }
                    break;
                }

                case 'c':
                    DisplayCharacter(va_arg(list, int));
                    chars++;
                    break;

                case 's':
                    chars += DisplayString(va_arg(list, char*));
                    break;

                case 'n':
                    switch (length) {
                    case 'H':
                        *(va_arg(list, char*)) = chars;
                        break;
                    case 'h':
                        *(va_arg(list, int*)) = chars;
                        break;
                    case 0: {
                        int* a = va_arg(list, int*);
                        *a = chars;
                        break;
                    }

                    case 'l':
                        *(va_arg(list, long*)) = chars;
                        break;
                    case 'q':
                        *(va_arg(list, long long*)) = chars;
                        break;
                    case 'j':
                        *(va_arg(list, intmax_t*)) = chars;
                        break;
                    case 't':
                        *(va_arg(list, uint64_t*)) = chars;
                        break;
                    default:
                        break;
                    }

                    break;

                default:
                    break;
                }
            } else {
                DisplayCharacter(format[i]);
                chars++;
            }
        }

        return chars;
    }

    int Print(const char* format, ...) {
        va_list args;
        va_start(args, format);
        int ret = Printv(format, args);
        va_end(args);
        return ret;
    }

    int Println(const char* format, ...) {
        va_list args;
        va_start(args, format);
        int ret = Printv(format, args);
        va_end(args);

        DisplayCharacter('\n');

        return ret + 1;
    }

    void SetForegroundColor(uint8_t red, uint8_t green, uint8_t blue) { foregroundColor = (red << 16) | (green << 8) | blue; }

    void SetBackgroundColor(uint8_t red, uint8_t green, uint8_t blue) { backgroundColor = (red << 16) | (green << 8) | blue; }

    void SetCursorPos(uint32_t x, uint32_t y) {
        if (x >= consoleWidth || y >= consoleHeight)
            return;

        cursorX = x;
        cursorY = y;
    }

    uint32_t GetCursorX() { return cursorX; }

    uint32_t GetCursorY() { return cursorY; }

    void ScrollUp() {
        if (!doubleBufferInit) {
            cursorX = 0;
            cursorY = 1;
            return;
        }

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

    void ClearScreen() {
        for (uint64_t i = 0; i < framebufferSize; i++)
            framebuffer[i] = backgroundColor;

        SetCursorPos(0, 1);
    }
} // namespace Console

void panic(const char* format, ...) {
    Console::SetForegroundColor(0xFF, 0x00, 0x00);
    Console::SetBackgroundColor(0x00, 0x00, 0x00);

    Console::Print("Panic: ");

    va_list args;
    va_start(args, format);
    Console::Printv(format, args);
    va_end(args);

    while (1)
        asm volatile("hlt");
}