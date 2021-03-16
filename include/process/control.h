#pragma once

#include <stdint.h>

#include <process/process.h>

void Yield();
void QueueExecution(Process* process);

uint64_t Execute(const char* filepath, const char** args, const char** env);

uint64_t Wait(uint64_t pid);

void Exit(uint64_t status);

int GetCurrentWorkingDirectory(void* ptr, uint64_t size);