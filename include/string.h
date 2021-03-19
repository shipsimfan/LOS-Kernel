#pragma once

#include <stdint.h>

void* memcpy(void* destination, const void* source, uint64_t num);
void* memset(void* ptr, int value, uint64_t num);
int memcmp(const void* str1, const void* str2, uint64_t n);

char* strcpy(char* destination, const char* source);
char* strncpy(char* destination, const char* source, uint64_t num);
char* strcat(char* destination, const char* source);
char* strncat(char* destination, const char* source, uint64_t num);

uint64_t strlen(const char* str);

int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, uint64_t n);

char tolower(char c);
