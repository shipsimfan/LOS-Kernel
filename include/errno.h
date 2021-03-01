#pragma once

#include <stdint.h>

// Standard Errors
#define SUCCESS 0x00

// Programmer Errors
#define ERROR_BAD_PARAMETER 0x01

// Other Errors
#define ERROR_ACPI_ERROR 0x100

extern uint64_t errno;