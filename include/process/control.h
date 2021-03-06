#pragma once

#include <stdint.h>

#include <process/process.h>

void Yield();
void QueueExecution(Process* process);

uint64_t Fork();
uint64_t Execute(const char* filepath);

uint64_t Wait(uint64_t pid);

void Exit(uint64_t status);