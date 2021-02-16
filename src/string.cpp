#include <string.h>

void* memcpy(void* destination, const void* source, uint64_t num) {
    int i = 0;
    if (num >= 8)
        for (; num >= 8; num -= 8, i++)
            ((uint64_t*)destination)[i] = ((uint64_t*)source)[i];

    i = i << 1;

    if (num >= 4) {
        ((uint32_t*)destination)[i] = ((uint32_t*)source)[i];
        num -= 4;
        i++;
    }

    i = i << 1;

    if (num >= 2) {
        ((uint16_t*)destination)[i] = ((uint16_t*)source)[i];
        num -= 2;
        i++;
    }

    i = i << 1;

    if (num == 1)
        ((uint8_t*)destination)[i] = ((uint8_t*)source)[i];

    return destination;
}

void* memset(void* ptr, int value, uint64_t num) {
    int i = 0;
    if (num >= 8)
        for (; num >= 8; num -= 8, i++)
            ((uint64_t*)ptr)[i] = 0;

    i = i << 1;

    if (num >= 4) {
        ((uint32_t*)ptr)[i] = 0;
        num -= 4;
        i++;
    }

    i = i << 1;

    if (num >= 2) {
        ((uint16_t*)ptr)[i] = 0;
        num -= 2;
        i++;
    }

    i = i << 1;

    if (num == 1)
        ((uint8_t*)ptr)[i] = 0;

    return ptr;
}

int memcmp(const void* str1, const void* str2, uint64_t n) {
    register const unsigned char* s1 = (const unsigned char*)str1;
    register const unsigned char* s2 = (const unsigned char*)str2;

    while (n-- > 0) {
        if (*s1++ != *s2++)
            return s1[-1] < s2[-1] ? -1 : 1;
    }

    return 0;
}

char* strcpy(char* destination, const char* source) {
    int i;
    for (i = 0; source[i]; i++)
        destination[i] = source[i];

    destination[i] = 0;
    return destination;
}

char* strcat(char* destination, const char* source) {
    int i;
    uint64_t len = strlen(destination);
    for (i = 0; source[i]; i++)
        destination[i + len] = source[i];

    destination[i] = 0;
    return destination;
}

uint64_t strlen(const char* str) {
    uint64_t i;
    for (i = 0; str[i]; i++)
        ;
    return i;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2))
        s1++, s2++;
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

char* strncpy(char* destination, const char* source, uint64_t num) {
    while (*source && num--) {
        *destination = *source;
        destination++;
        source++;
    }

    *destination = '\0';

    return destination;
}

char* strncat(char* destination, const char* source, uint64_t num) {
    uint64_t i, j;
    for (i = 0; destination[i] != '\0'; i++)
        ;

    for (j = 0; source[j] != '\0' && j < num; j++)
        destination[i + j] = source[j];

    destination[i + j] = '\0';
    return destination;
}

int strncmp(const char* s1, const char* s2, uint64_t n) {
    while (n--)
        if (*s1++ != *s2++)
            return *(unsigned char*)(s1 - 1) - *(unsigned char*)(s2 - 1);
    return 0;
}