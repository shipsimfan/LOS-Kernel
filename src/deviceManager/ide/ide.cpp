#include <dev/ide.h>

#include <dev.h>
#include <dev/pci.h>
#include <interrupt.h>
#include <logger.h>
#include <stdlib.h>

#define IDE_SR_BSY 0x80
#define IDE_SR_DRDY 0x40
#define IDE_SR_DF 0x20
#define IDE_SR_DSC 0x10
#define IDE_SR_DRQ 0x08
#define IDE_SR_CORR 0x04
#define IDE_SR_IDX 0x02
#define IDE_ST_ERR 0x01

#define IDE_ER_BBK 0x80
#define IDE_ER_UNC 0x40
#define IDE_ER_MC 0x20
#define IDE_ER_IDNF 0x10
#define IDE_ER_MCR 0x08
#define IDE_ER_ABRT 0x04
#define IDE_ER_TK0NF 0x02
#define IDE_ER_AMNF 0x01

#define ATA_CMD_READ_PIO 0x20
#define ATA_CMD_READ_PIO_EXT 0x24
#define ATA_CMD_READ_DMA 0xC8
#define ATA_CMD_READ_DMA_EXT 0x25
#define ATA_CMD_WRITE_PIO 0x30
#define ATA_CMD_WRITE_PIO_EXT 0x34
#define ATA_CMD_WRITE_DMA 0xCA
#define ATA_CMD_WRITE_DMA_EXT 0x35
#define ATA_CMD_CACHE_FLUSH 0xE7
#define ATA_CMD_CACHE_FLUSH_EXT 0xEA
#define ATA_CMD_PACKET 0xA0
#define ATA_CMD_IDENTIFY_PACKET 0xA1
#define ATA_CMD_IDENTIFY 0xEC

#define ATAPI_CMD_READ 0xA8
#define ATAPI_CMD_EJECT 0x1B

#define ATA_IDENT_DEVICETYPE 0
#define ATA_IDENT_CYLIDNERS 2
#define ATA_IDENT_HEADS 6
#define ATA_IDENT_SECTORS 12
#define ATA_IDENT_SERIAL 20
#define ATA_IDENT_MODEL 54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID 106
#define ATA_IDENT_MAX_LBA 120
#define ATA_IDENT_COMMANDSETS 164
#define ATA_IDENT_MAX_LBA_EXT 200

#define ATA_MASTER 0x00
#define ATA_SLAVE 0x01

#define IDE_ATA 0x0
#define IDE_ATAPI 0x01

#define ATA_REG_DATA 0x00
#define ATA_REG_ERROR 0x01
#define ATA_REG_FEATURES 0x01
#define ATA_REG_SECCOUNT0 0x02
#define ATA_REG_LBA0 0x03
#define ATA_REG_LBA1 0x04
#define ATA_REG_LBA2 0x05
#define ATA_REG_HDDEVSEL 0x06
#define ATA_REG_COMMAND 0x07
#define ATA_REG_STATUS 0x07
#define ATA_REG_SECCOUNT1 0x08
#define ATA_REG_LBA3 0x09
#define ATA_REG_LBA4 0x0A
#define ATA_REG_LBA5 0x0B
#define ATA_REG_CONTROL 0x0C
#define ATA_REG_ALTSTATUS 0x0C
#define ATA_REG_DEVADDRESS 0x0D

#define ATA_PRIMARY 0x00
#define ATA_SECONDARY 0x01

#define ATA_READ 0x00
#define ATA_WRITE 0x01

namespace DeviceManager { namespace IDE {
    DeviceDriver IDEDriver;
    DeviceDriver ATADriver;

    uint8_t irqInvoked = 0;
    uint8_t atapiPacket[12] = {0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    extern "C" void IRQ14();
    extern "C" void IRQ15();

    extern "C" void IRQ14Handler() {}

    extern "C" void IRQ15Handler() {}

    void WriteRegister(IDEDeviceInfo* ideDevice, uint8_t channel, uint8_t reg, uint8_t data) {
        if (reg > 0x07 && reg < 0x0C)
            WriteRegister(ideDevice, channel, ATA_REG_CONTROL, 0x80 | ideDevice->channels[channel].nIEN);

        if (reg < 0x08)
            outb(ideDevice->channels[channel].IO + reg, data);
        else if (reg < 0x0C)
            outb(ideDevice->channels[channel].IO + reg - 0x06, data);
        else if (reg < 0x0E)
            outb(ideDevice->channels[channel].control + reg - 0x0A, data);
        else if (reg < 0x16)
            outb(ideDevice->channels[channel].busMaster + reg - 0x0E, data);

        if (reg > 0x07 && reg < 0x0C)
            WriteRegister(ideDevice, channel, ATA_REG_CONTROL, ideDevice->channels[channel].nIEN);
    }

    uint8_t ReadRegister(IDEDeviceInfo* ideDevice, uint8_t channel, uint8_t reg) {
        uint8_t result;
        if (reg > 0x07 && reg < 0x0C)
            WriteRegister(ideDevice, channel, ATA_REG_CONTROL, 0x80 | ideDevice->channels[channel].nIEN);

        if (reg < 0x08)
            result = inb(ideDevice->channels[channel].IO + reg);
        else if (reg < 0x0C)
            result = inb(ideDevice->channels[channel].IO + reg - 0x06);
        else if (reg < 0x0E)
            result = inb(ideDevice->channels[channel].control + reg - 0x0A);
        else if (reg < 0x16)
            result = inb(ideDevice->channels[channel].busMaster + reg - 0x0E);

        if (reg > 0x07 && reg < 0x0C)
            WriteRegister(ideDevice, channel, ATA_REG_CONTROL, ideDevice->channels[channel].nIEN);

        return result;
    }

    bool ATAVerifyDevice(Device* ideDevice) { return true; }

    void ATARegisterDevice(Device* device) {
        IDEDeviceInfo* ideDevice = (IDEDeviceInfo*)device->driverInfo;

        // Disable IRQs
        WriteRegister(ideDevice, ATA_PRIMARY, ATA_REG_CONTROL, 2);
        WriteRegister(ideDevice, ATA_SECONDARY, ATA_REG_CONTROL, 2);

        // Check the connected drives
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                uint8_t err = 0;
                uint8_t type = IDE_ATA;
                uint8_t status;

                // Select Drive
                WriteRegister(ideDevice, i, ATA_REG_HDDEVSEL, 0xA0 | (j << 4));
            }
        }
    }

    void IDERegisterChildDriver(DeviceDriver* driver) {
        Device* device = IDEDriver.deviceHead;
        while (device != nullptr)
            device = RegisterDevice(device, driver);
    }

    bool IDEVerifyDevice(Device* device) {
        PCI::PCIDeviceInfo* devInfo = (PCI::PCIDeviceInfo*)device->driverInfo;

        return devInfo->classCode == 1 && devInfo->subClass == 1;
    }

    void IDERegisterDevice(Device* device) {
        PCI::StandardPCIDeviceInfo* devInfo = (PCI::StandardPCIDeviceInfo*)device->driverInfo;

        IDEDeviceInfo* ideInfo = (IDEDeviceInfo*)malloc(sizeof(IDEDeviceInfo));
        device->driverInfo = ideInfo;

        ideInfo->channels[ATA_PRIMARY].IO = devInfo->baseAddr0;
        ideInfo->channels[ATA_PRIMARY].control = devInfo->baseAddr1;
        ideInfo->channels[ATA_PRIMARY].busMaster = devInfo->baseAddr4;
        ideInfo->channels[ATA_SECONDARY].IO = devInfo->baseAddr2;
        ideInfo->channels[ATA_SECONDARY].control = devInfo->baseAddr3;
        ideInfo->channels[ATA_SECONDARY].busMaster = devInfo->baseAddr4 + 8;

        if (ideInfo->channels[ATA_PRIMARY].IO == 0 || ideInfo->channels[ATA_PRIMARY].IO == 1)
            ideInfo->channels[ATA_PRIMARY].IO = 0x1F0;

        if (ideInfo->channels[ATA_PRIMARY].control == 0 || ideInfo->channels[ATA_PRIMARY].control == 1)
            ideInfo->channels[ATA_PRIMARY].control = 0x3F4;

        if (ideInfo->channels[ATA_SECONDARY].IO == 0 || ideInfo->channels[ATA_SECONDARY].IO == 0)
            ideInfo->channels[ATA_SECONDARY].IO = 0x170;

        if (ideInfo->channels[ATA_SECONDARY].control == 0 || ideInfo->channels[ATA_SECONDARY].control == 1)
            ideInfo->channels[ATA_SECONDARY].control = 0x374;

        PCI::WriteConfigB(devInfo->bus, devInfo->device, devInfo->function, PCI_CONFIG_INT_LINE, 0xFE);
        uint8_t intLine = PCI::ReadConfigB(devInfo->bus, devInfo->device, devInfo->function, PCI_CONFIG_INT_LINE);
        if (intLine == 0xFE) {
            ideInfo->irq1 = InterruptHandler::SetAvailableIRQ(IRQ14);
            ideInfo->irq2 = InterruptHandler::SetAvailableIRQ(IRQ15);

            if (ideInfo->irq1 == 0xFF || ideInfo->irq2 == 0xFF) {
                errorLogger.Log("Failed to set IRQS (1: %i, 2: %i)", ideInfo->irq1, ideInfo->irq2);
                free(devInfo);
                return;
            }

            PCI::WriteConfigB(devInfo->bus, devInfo->device, devInfo->function, PCI_CONFIG_INT_LINE, ideInfo->irq1);
            PCI::WriteConfigB(devInfo->bus, devInfo->device, devInfo->function, PCI_CONFIG_INT_PIN, ideInfo->irq2);

            debugLogger.Log("IDE IRQs set to %i and %i", ideInfo->irq1, ideInfo->irq2);
        } else {
            if (devInfo->progIF == 0x8A || devInfo->progIF == 0x80) {
                debugLogger.Log("Standard parallel IDE device");
                ideInfo->irq1 = 14;
                ideInfo->irq2 = 15;
                InterruptHandler::SetIRQ(14, IRQ14);
                InterruptHandler::SetIRQ(15, IRQ15);
            } else {
                errorLogger.Log("IDE device doesn't use IRQs");
                ideInfo->irq1 = 0xFF;
                ideInfo->irq2 = 0xFF;
                free(devInfo);
                return;
            }
        }

        free(devInfo);
    }

    void RegisterIDEDriver() {
        IDEDriver.signature = DEVICE_DRIVER_SIGNATURE_IDE;
        IDEDriver.name = "LOS IDE Driver";
        IDEDriver.parent = GetDeviceDriver(DEVICE_DRIVER_SIGNATURE_PCI);
        IDEDriver.RegisterChildDriver = IDERegisterChildDriver;
        IDEDriver.VerifyDevice = IDEVerifyDevice;
        IDEDriver.RegisterDevice = IDERegisterDevice;
        RegisterDeviceDriver(&IDEDriver);

        ATADriver.signature = DEVICE_DRIVER_SIGNATURE_ATA;
        ATADriver.name = "LOS IDE-ATA Driver";
        ATADriver.parent = &IDEDriver;
        ATADriver.VerifyDevice = ATAVerifyDevice;
        ATADriver.RegisterDevice = ATARegisterDevice;
        // RegisterDeviceDriver(&ATADriver);
    }
}} // namespace DeviceManager::IDE
