#pragma once

#include <filesystem/driver.h>
#include <stdint.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define OPEN_READ 1
#define OPEN_WRITE 2

int Open(const char* filepath, int flags);
void Close(int fd);

int64_t Read(int fd, void* buffer, int64_t count);
int64_t Write(int fd, void* buffer, int64_t count);

int64_t Seek(int fd, int64_t offset, int whence);
int64_t Tell(int fd);

int GetNumFilesystems();
Directory* GetRootDirectory(int filesystem);

uint64_t ChangeDirectory(const char* path);