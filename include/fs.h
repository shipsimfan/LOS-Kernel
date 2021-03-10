#pragma once

#include <stdint.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

int Open(const char* filepath);
void Close(int fd);

int64_t Read(int fd, void* buffer, int64_t count);

int64_t Seek(int fd, int64_t offset, int whence);
int64_t Tell(int fd);