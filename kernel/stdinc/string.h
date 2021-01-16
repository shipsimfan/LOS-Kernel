#pragma once

#include <types.h>

void* memcpy(void* destination, const void* source, size_t num);

void* memset(void* ptr, int value, size_t num);

char* strcpy(char* destination, const char* source);

char* strcat(char* destination, const char* source);

size_t strlen(const char* str);