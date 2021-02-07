#pragma once

#include <stdint.h>

class Logger {
public:
    enum TYPE { INFORMATION = 0, DEBUGGER = 1, WARNING = 2, ERROR = 3 };

    void Set(const char* name, TYPE type, uint32_t color);

    int Log(const char* format, ...);

private:
    uint32_t color;
    TYPE type;
    const char* name;
};

extern Logger infoLogger;
extern Logger debugLogger;
extern Logger warningLogger;
extern Logger errorLogger;