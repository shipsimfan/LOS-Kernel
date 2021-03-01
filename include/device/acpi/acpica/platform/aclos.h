#pragma once

#define ACPI_USE_DO_WHILE_0
#define ACPI_IGNORE_PACKAGE_RESOLUTION_ERRORS

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

#define ACPI_MACHINE_WIDTH 64
#define COMPILER_DEPENDENT_INT64 long
#define COMPILER_DEPENDENT_UINT64 unsigned long

#ifndef __cdecl
#define __cdecl
#endif

#define ACPI_CACHE_T ACPI_MEMORY_LIST
#define ACPI_USE_LOCAL_CACHE 1

#define ACPI_CREATE_PREDEFINED_TABLE

#define FILE void