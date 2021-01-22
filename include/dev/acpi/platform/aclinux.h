#pragma once

#define ACPI_USE_SYSTEM_CLIBRARY
#define ACPI_USE_DO_WHILE_0
#define ACPI_IGNORE_PACKAGE_RESOLUTION_ERRORS

#define ACPI_USE_STANDARD_HEADERS

#ifdef ACPI_USE_STANDARD_HEADERS
#include <unistd.h>
#endif

/* Define/disable kernel-specific declarators */

#ifndef __init
#define __init
#endif
#ifndef __iomem
#define __iomem
#endif

/* Host-dependent types and defines for user-space ACPICA */

#define ACPI_FLUSH_CPU_CACHE()
#define ACPI_CAST_PTHREAD_T(Pthread) ((ACPI_THREAD_ID)(Pthread))

#if defined(__ia64__) || (defined(__x86_64__) && !defined(__ILP32__)) || \
    defined(__aarch64__) || defined(__PPC64__) ||                        \
    defined(__s390x__) ||                                                \
    (defined(__riscv) && (defined(__LP64__) || defined(_LP64)))
#define ACPI_MACHINE_WIDTH 64
#define COMPILER_DEPENDENT_INT64 long
#define COMPILER_DEPENDENT_UINT64 unsigned long
#else
#define ACPI_MACHINE_WIDTH 32
#define COMPILER_DEPENDENT_INT64 long long
#define COMPILER_DEPENDENT_UINT64 unsigned long long
#define ACPI_USE_NATIVE_DIVIDE
#define ACPI_USE_NATIVE_MATH64
#endif

#ifndef __cdecl
#define __cdecl
#endif

#define ACPI_CACHE_T ACPI_MEMORY_LIST
#define ACPI_USE_LOCAL_CACHE 1