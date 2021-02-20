#pragma once

#include <stdint.h>

uint64_t Fork();

uint64_t Wait(uint64_t pid);

void Exit(uint64_t status);