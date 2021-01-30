#include <dev/hpet.h>

#include <interrupt.h>
#include <logger.h>
#include <mem/defs.h>
#include <mem/virtual.h>
#include <stdlib.h>

#define HPET_GENERAL_CAPABILITIES_REG 0x00
#define HPET_GENERAL_CONFIG_REG (0x10 / 8)
#define HPET_GENERAL_INTERRUPT_REG (0x20 / 8)
#define HPET_MAIN_COUNTER_REG (0xF0 / 8)
#define HPET_TIMER_CONFIG_REG(n) ((0x100 + 0x20 * n) / 8)
#define HPET_TIMER_COMPARE_REG(n) ((0x108 + 0x20 * n) / 8)
#define HPET_TIMER_FSB_INTERRUPT_REG(n) ((0x110 + 0x20 * n) / 8)

#define TIMER_IRQ 0

namespace DeviceManager { namespace HPET {
#pragma pack(push)
#pragma pack(1)

    struct HPETTable {
        char signature[4];
        uint32_t length;
        uint8_t revision;
        uint8_t checksum;
        char oemID[6];
        uint64_t oemTableID;
        uint32_t oemRevision;
        uint32_t creatorID;
        uint32_t creatorRevision;

        uint8_t hardwareRevisionID;
        uint8_t comparatorCount : 5;
        uint8_t counterSize : 1;
        uint8_t reserved1 : 1;
        uint8_t legacyReplacement : 1;
        uint16_t vendorID;
        uint8_t addressSpace;
        uint8_t registerBitWidth;
        uint8_t registerBitOffset;
        uint8_t reserved2;
        uint64_t address;
        uint8_t hpetNumber;
        uint16_t minimumTick;
        uint8_t pageProtection;
    };

#pragma pack(pop)

    uint64_t* hpet;
    uint32_t minimumTick;

    uint64_t currentTimeMillis = 0;

    extern "C" void HPETTimerIRQ() { currentTimeMillis++; }

    extern "C" void sleep(uint64_t milliseconds) {
        // Blocking sleep function
        uint64_t startTime = currentTimeMillis;
        uint64_t endTime = startTime + milliseconds;
        while (currentTimeMillis < endTime)
            ;
    }

    extern "C" uint64_t getCurrentTimeMillis() { return currentTimeMillis; }

    bool RegisterHPETDriver(void* hpetTable) {
        if (hpetTable == nullptr) {
            errorLogger.Log("Invalid HPET table!");
            return false;
        }

        HPETTable* table = (HPETTable*)hpetTable;

        if (table->addressSpace != 0) {
            errorLogger.Log("Invalid HPET address space! (%i)", table->addressSpace);
            return false;
        }

        // Find HPET base
        hpet = (uint64_t*)(table->address + KERNEL_VMA);
        MemoryManager::Virtual::AllocatePage(hpet, table->address, true);

        // Disable HPET
        hpet[HPET_GENERAL_CONFIG_REG] = 0;

        // Save minimum tick from config register
        minimumTick = hpet[HPET_GENERAL_CAPABILITIES_REG] >> 32 & 0xFFFFFFFF;

        // Setup IRQ
        InterruptHandler::SetIRQ(TIMER_IRQ, HPETTimerIRQ);

        // Initialize timers
        uint8_t numTimers = ((hpet[HPET_GENERAL_CAPABILITIES_REG] >> 8) & 0x1F) + 1;
        for (int i = 0; i < numTimers; i++) {
            uint64_t timerCap = hpet[HPET_TIMER_CONFIG_REG(i)];
            timerCap &= ~(1 << 14); // Disable FSB
            timerCap &= ~(1 << 3);  // Disable Periodic
            timerCap &= ~(1 << 2);  // Disable Interrupts
            hpet[HPET_TIMER_CONFIG_REG(i)] = timerCap;
        }

        // Start the timer
        uint64_t timerVal = 1000000000000 / minimumTick;
        hpet[HPET_GENERAL_CONFIG_REG] = 3;

        // Start the millisecond clock
        hpet[HPET_TIMER_CONFIG_REG(0)] = TIMER_IRQ | (1 << 2) | (1 << 3) | (1 << 6);
        hpet[HPET_TIMER_COMPARE_REG(0)] = hpet[HPET_MAIN_COUNTER_REG] + timerVal;
        hpet[HPET_TIMER_COMPARE_REG(0)] = timerVal;

        return true;
    }
}} // namespace DeviceManager::HPET