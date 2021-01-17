#include <stdio.h>

#include <console.h>
#include <stdarg.h>
#include <string.h>

int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vprintf(format, args);
    va_end(args);
    return ret;
}

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

bool IsDigit(char c) { return c >= '0' && c <= '9'; }

int vprintf(const char* format, va_list list) {
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
                    Console::DisplayCharacter('0');
                    chars++;
                }
            }

            if (specifier == 'p') {
                base = 16;
                length = 'z';
                specifier = 'u';
            }

            switch (specifier) {
            case 'X':
                base = 16;
            case 'x':
                base = base == 10 ? 17 : base;
                if (altForm)
                    chars += Console::DisplayString("0x");

            case 'u': {
                switch (length) {
                case 0: {
                    unsigned int integer = va_arg(list, unsigned int);
                    __uint_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    chars += Console::DisplayString(intStrBuffer);
                    break;
                }
                case 'H': {
                    unsigned char integer = va_arg(list, unsigned int);
                    __uint_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    chars += Console::DisplayString(intStrBuffer);
                    break;
                }
                case 'h': {
                    unsigned short integer = va_arg(list, unsigned int);
                    __uint_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    chars += Console::DisplayString(intStrBuffer);
                    break;
                }
                case 'l': {
                    unsigned long integer = va_arg(list, unsigned long);
                    __uint_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    chars += Console::DisplayString(intStrBuffer);
                    break;
                }
                case 'q': {
                    unsigned long long integer = va_arg(list, unsigned long long);
                    __uint_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    chars += Console::DisplayString(intStrBuffer);
                    break;
                }
                case 'j': {
                    uintmax_t integer = va_arg(list, uintmax_t);
                    __uint_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    chars += Console::DisplayString(intStrBuffer);
                    break;
                }
                case 'z': {
                    size_t integer = va_arg(list, size_t);
                    __uint_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    chars += Console::DisplayString(intStrBuffer);
                    break;
                }
                case 't': {
                    ptrdiff_t integer = va_arg(list, ptrdiff_t);
                    __uint_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    chars += Console::DisplayString(intStrBuffer);
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
                    chars += Console::DisplayString(intStrBuffer);
                    break;
                }
                case 'H': {
                    char integer = va_arg(list, int);
                    __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    chars += Console::DisplayString(intStrBuffer);
                    break;
                }
                case 'h': {
                    short integer = va_arg(list, int);
                    __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    chars += Console::DisplayString(intStrBuffer);
                    break;
                }
                case 'l': {
                    long integer = va_arg(list, long);
                    __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    chars += Console::DisplayString(intStrBuffer);
                    break;
                }
                case 'q': {
                    long long integer = va_arg(list, long long);
                    __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    chars += Console::DisplayString(intStrBuffer);
                    break;
                }
                case 'j': {
                    intmax_t integer = va_arg(list, intmax_t);
                    __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    chars += Console::DisplayString(intStrBuffer);
                    break;
                }
                case 'z': {
                    size_t integer = va_arg(list, size_t);
                    __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    chars += Console::DisplayString(intStrBuffer);
                    break;
                }
                case 't': {
                    ptrdiff_t integer = va_arg(list, ptrdiff_t);
                    __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                    chars += Console::DisplayString(intStrBuffer);
                    break;
                }
                default:
                    break;
                }
                break;
            }

            case 'c':
                Console::DisplayCharacter(va_arg(list, int));
                chars++;
                break;

            case 's':
                chars += Console::DisplayString(va_arg(list, char*));
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
                    *(va_arg(list, ptrdiff_t*)) = chars;
                    break;
                default:
                    break;
                }

                break;

            default:
                break;
            }
        } else {
            Console::DisplayCharacter(format[i]);
            chars++;
        }
    }

    return chars;
}
