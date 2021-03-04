#include <console.h>

#include <bootloader.h>
#include <errno.h>
#include <panic.h>
#include <stdarg.h>
#include <string.h>

namespace Console {
    Device::Device* videoDevice = nullptr;

    void SetVideoDevice(Device::Device* device) { videoDevice = device; }

    void DisplayCharacter(char character) {
        if (videoDevice == nullptr)
            return;

        videoDevice->WriteStream(0, &character, 1);
    }

    int DisplayString(const char* str) {
        if (videoDevice == nullptr)
            return 0;

        return videoDevice->WriteStream(0, (void*)str, INT32_MAX);
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
        if (videoDevice != nullptr) {
            va_list args;
            va_start(args, format);
            videoDevice->Open();
            int ret = Printv(format, args);
            videoDevice->Close();
            va_end(args);
            return ret;
        }

        return 0;
    }

    int Println(const char* format, ...) {
        if (videoDevice != nullptr) {
            va_list args;
            va_start(args, format);
            videoDevice->Open();
            int ret = Printv(format, args);
            va_end(args);
            DisplayCharacter('\n');
            videoDevice->Close();
            return ret + 1;
        }

        return 0;
    }

    void SetForegroundColor(uint8_t red, uint8_t green, uint8_t blue) {
        if (videoDevice != nullptr)
            videoDevice->Write(FOREGROUND_COLOR_ADDRESS, (red << 16) | (green << 8) | blue);
    }

    void SetBackgroundColor(uint8_t red, uint8_t green, uint8_t blue) {
        if (videoDevice != nullptr)
            videoDevice->Write(BACKGROUND_COLOR_ADDRESS, (red << 16) | (green << 8) | blue);
    }

    void SetCursorPos(uint32_t x, uint32_t y) {
        if (videoDevice != nullptr) {
            videoDevice->Write(CURSOR_X_ADDRESS, x);
            videoDevice->Write(CURSOR_Y_ADDRESS, y);
        }
    }
    uint32_t GetCursorX() {
        if (videoDevice != nullptr) {
            uint64_t val;
            if (videoDevice->Read(CURSOR_X_ADDRESS, &val) == SUCCESS)
                return val;
        }

        return 0;
    }
    uint32_t GetCursorY() {
        if (videoDevice != nullptr) {
            uint64_t val;
            if (videoDevice->Read(CURSOR_Y_ADDRESS, &val) == SUCCESS)
                return val;
        }

        return 0;
    }

    void ClearScreen() {
        if (videoDevice != nullptr)
            videoDevice->Write(CLEAR_SCREEN_ADDRESS, 0);
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