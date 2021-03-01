#include <device/hpet.h>

#include <console.h>
#include <device/acpi/acpi.h>
#include <interrupt/irq.h>
#include <memory/virtual.h>
#include <panic.h>

#include "hpet.h"

uint64_t currentTimeMillis;

void TimerIRQHandler(void* context) {
    currentTimeMillis++;
    if (currentTimeMillis % 1000 == 0)
        Console::Println("Current System Time: %is", currentTimeMillis / 1000);
}

HPET::HPET() {
    // Get the HPET table
    ACPI::HPET* table = (ACPI::HPET*)ACPI::GetTable(HPET_SIGNATURE);
    if (table == nullptr)
        panic("Failed to get HPET table!");

    // Verify address space
    if (table->address.space != 0)
        panic("Invalid HPET address space (%i)", table->address.space);

    // Save and allocate hpet address
    address = (uint64_t*)(table->address.address + KERNEL_VMA);
    Memory::Virtual::Allocate(address, table->address.address);

    // Disable HPET
    address[HPET_GENERAL_CONFIG_REG] = 0;

    // Save minimum tick
    uint32_t minimumTick = address[HPET_GENERAL_CAPABILITIES_REG] >> 32 & 0xFFFFFFFF;

    // Setup IRQ
    if (!Interrupt::InstallIRQHandler(TIMER_IRQ, TimerIRQHandler, nullptr))
        panic("Failed to install timer IRQ!");

    // Initialize timers
    uint8_t numTimers = ((address[HPET_GENERAL_CAPABILITIES_REG] >> 8) & 0x01) + 1;
    for (uint8_t i = 0; i < numTimers; i++) {
        uint64_t timerCap = address[HPET_TIMER_CONFIG_REG(i)];
        timerCap &= ~(1 << 14); // Disable FSB
        timerCap &= ~(1 << 3);  // Disable Periodic
        timerCap &= ~(1 << 2);  // Disable Interrupts
        address[HPET_TIMER_CONFIG_REG(i)] = timerCap;
    }

    // Start the timer
    uint64_t timerVal = 1000000000000 / minimumTick;
    address[HPET_GENERAL_CONFIG_REG] = 3;

    // Start the millisecond clock
    address[HPET_TIMER_CONFIG_REG(0)] = TIMER_IRQ | (1 << 2) | (1 << 3) | (1 << 6);
    address[HPET_TIMER_COMPARE_REG(0)] = address[HPET_MAIN_COUNTER_REG] + timerVal;
    address[HPET_TIMER_COMPARE_REG(0)] = timerVal;
}