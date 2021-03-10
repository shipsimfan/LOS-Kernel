#pragma once

#include <process/control.h>

// Standard Errors
#define SUCCESS 0x00

#define ERROR_TIMEOUT 0x01

// Programmer Errors
#define ERROR_BAD_PARAMETER 0x01
#define ERROR_NOT_IMPLEMENTED 0x02
#define ERROR_OUT_OF_RANGE 0x03

// Synchronization Errors
#define ERROR_NOT_OWNER 0x10

// Other Errors
#define ERROR_ACPI_ERROR 0x20
#define ERROR_DEVICE_ERROR 0x21
#define ERROR_READ_ONLY 0x22

#define errno currentProcess->errno