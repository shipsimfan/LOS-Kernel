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
    struct HPETTimerInfo {
        uint32_t interruptRouteCapability;
        bool fsbCapable;
        bool fsbEnabled;
        uint8_t irq;
        bool is64Bit;
        bool periodicCapable;
        bool periodicEnabled;
        bool interruptsEnabled;
    };

    uint64_t* hpet;
    uint32_t minimumTick;

    HPETTimerInfo* timers;
    uint8_t numTimers;

    uint64_t currentTimeMillis = 0;

    extern "C" void HPETTimerIRQHandler();

    extern "C" void HPETTimerIRQ() {
        currentTimeMillis++;

        InterruptHandler::WriteEOI();
    }

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

        // Initialize timers
        numTimers = ((hpet[HPET_GENERAL_CAPABILITIES_REG] >> 8) & 0x1F) + 1;
        timers = (HPETTimerInfo*)malloc(sizeof(HPETTimerInfo) * numTimers);
        for (int i = 0; i < numTimers; i++) {
            uint64_t timerCap = hpet[HPET_TIMER_CONFIG_REG(i)];
            timers[i].interruptRouteCapability = (timerCap >> 32) & 0xFFFFFFFF;
            timers[i].fsbCapable = (timerCap >> 15) & 1;
            timers[i].fsbEnabled = false;
            timerCap &= ~(1 << 14);
            timers[i].is64Bit = (timerCap >> 8) & 1;
            timers[i].periodicCapable = (timerCap >> 4) & 1;
            timers[i].periodicEnabled = false;
            timerCap &= ~(1 << 3);
            timers[i].interruptsEnabled = false;
            timerCap &= ~(1 << 2);

            // Select first irq
            for (int j = 0; j < 32; j++) {
                if (((timers[i].interruptRouteCapability >> j) & 1) == 1) {
                    timers[i].irq = j;

                    timerCap &= ~(0xF << 9);
                    timerCap |= (timers[i].irq & 0xF) << 9;

                    break;
                }
            }

            if ((((timers[i].interruptRouteCapability) >> timers[i].irq) & 1) == 0) {
                errorLogger.Log("Unable to get IRQ");
                return false;
            }

            InterruptHandler::SetIRQ(timers[i].irq, HPETTimerIRQHandler, false, (timerCap >> 1) & 1);

            hpet[HPET_TIMER_CONFIG_REG(i)] = timerCap;
        }

        // Start the timer
        uint64_t timerVal = 1000000000000 / minimumTick;
        hpet[HPET_GENERAL_CONFIG_REG] = 1;

        // Start the millisecond clock
        InterruptHandler::SetIRQMask(timers[0].irq, false);
        hpet[HPET_TIMER_CONFIG_REG(0)] = (timers[0].irq << 9) | (1 << 2) | (1 << 3) | (1 << 6);
        hpet[HPET_TIMER_COMPARE_REG(0)] = hpet[HPET_MAIN_COUNTER_REG] + timerVal;
        hpet[HPET_TIMER_COMPARE_REG(0)] = timerVal;

        return true;
    }
}} // namespace DeviceManager::HPET