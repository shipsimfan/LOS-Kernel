#include <semaphore.h>

Semaphore::Semaphore(uint64_t initialValue, uint64_t maxValue) : value(initialValue), maxValue(maxValue) {}

void Semaphore::Signal() {
    value++;
    if (value > maxValue)
        value = maxValue;
}

void Semaphore::Wait() {
    while (value == 0)
        asm volatile("pause");

    value--;
}