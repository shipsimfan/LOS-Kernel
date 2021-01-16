#pragma once

class Logger {
public:
    enum COLOR { BLACK = 0, BLUE = 1, GREEN = 2, AQUA = 3, RED = 4, PURPLE = 5, YELLOW = 6, WHITE = 7, GRAY = 8, LIGHT_BLUE = 9, LIGHT_GREEN = 10, LIGHT_AQUA = 11, LIGHT_RED = 12, LIGHT_PURPLE = 13, LIGHT_YELLOW = 14, BRIGHT_WHITE = 15 };
    enum TYPE { INFORMATION = 0, DEBUGGER = 1, WARNING = 2, ERROR = 3 };

    void Set(const char* name, TYPE type, COLOR color);

    int Log(const char* format, ...);

private:
    COLOR color;
    TYPE type;
    const char* name;
};

extern Logger infoLogger;
extern Logger debugLogger;
extern Logger warningLogger;
extern Logger errorLogger;