#include <logger.h>

#include <console.h>
#include <stdio.h>

const char* typeStrings[] = {"INFO", "DEBUG", "WARN", "ERROR"};

void Logger::Set(const char* name, TYPE type, uint32_t color) {
    this->name = name;
    this->color = color;
    this->type = type;
}

int Logger::Log(const char* format, ...) {
#ifndef DEBUG
    if (type == DEBUGGER)
        return 0;
#endif

    Console::SetForegroundColor(color);
    int ret = printf("[%s] [%s] ", typeStrings[type], name);

    va_list args;
    va_start(args, format);
    ret += vprintf(format, args);
    va_end(args);

    ret += printf("\n");

    Console::SetForegroundColor(0xFFFFFFFF);

    return ret;
}