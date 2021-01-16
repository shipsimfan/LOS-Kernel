#include <console.h>

#define CONSOLE_WIDTH 80
#define CONSOLE_HEIGHT 25

namespace Console {
    volatile char* videoMemory = (volatile char*)0xFFFF8000000B8000;

    int consoleX = 0;
    int consoleY = 0;

    uint8_t color = 0x07;

    void ScrollUp() {
        int index = 0;
        for (int y = 0; y < CONSOLE_HEIGHT - 1; y++) {
            for (int x = 0; x < CONSOLE_WIDTH; x++) {
                videoMemory[index] = videoMemory[index + (CONSOLE_WIDTH * 2)];
                videoMemory[index + 1] = videoMemory[index + (CONSOLE_WIDTH * 2) + 1];
                index += 2;
            }
        }

        for (int x = 0; x < CONSOLE_WIDTH; x++) {
            videoMemory[index++] = ' ';
            videoMemory[index++] = color;
        }

        consoleX = 0;
        consoleY = CONSOLE_HEIGHT - 1;
    }

    void DisplayCharacter(char character) {
        if (character == '\n') { // Newline character
            consoleX = 0;
            consoleY++;

            if (consoleY >= CONSOLE_HEIGHT)
                ScrollUp();
        } else {
            int index = (consoleX + consoleY * CONSOLE_WIDTH) * 2;
            videoMemory[index] = character;
            videoMemory[index + 1] = color;

            consoleX++;
            if (consoleX >= CONSOLE_WIDTH) {
                consoleX = 0;
                consoleY++;

                if (consoleY == 0)
                    ScrollUp();
            }
        }
    }

    int DisplayString(const char* string) {
        int i;
        for (i = 0; string[i]; i++)
            DisplayCharacter(string[i]);

        return i;
    }

    void SetForegroundColor(uint8_t fgColor) { color = (color & 0xF0) | (fgColor & 0x0F); }
    void SetBackgroundColor(uint8_t bgColor) { color = (color & 0x0F) | ((bgColor << 4) & 0xF0); }

    void ClearScreen() {
        consoleX = 0;
        consoleY = 0;

        for (int i = 0; i < CONSOLE_WIDTH * CONSOLE_HEIGHT; i++)
            DisplayCharacter(' ');

        consoleX = 0;
        consoleY = 0;
    }
} // namespace Console