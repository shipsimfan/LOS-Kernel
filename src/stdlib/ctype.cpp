#include <ctype.h>

extern "C" int tolower(int argument) {
    if (argument >= 'A' && argument <= 'Z')
        return argument - ('A' - 'a');

    return argument;
}

extern "C" int toupper(int argument) {
    if (argument >= 'a' && argument <= 'z')
        return argument + ('A' - 'a');

    return argument;
}

extern "C" int isspace(int argument) { return argument == ' ' ? 1 : 0; }

extern "C" int isxdigit(int argument) { return (argument >= '0' && argument <= '9') || (argument >= 'A' && argument <= 'F') ? 1 : 0; }

extern "C" int isdigit(int argument) { return argument >= '0' && argument <= '9' ? 1 : 0; }

extern "C" int isprint(int argument) { return argument != 0 && argument != '\n' ? 1 : 0; }