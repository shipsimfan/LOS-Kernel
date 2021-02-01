#include <dev/ide.h>

#include <dev.h>
#include <dev/pci.h>
#include <fs.h>
#include <interrupt.h>
#include <kernel/time.h>
#include <logger.h>
#include <stdlib.h>

#define IDE_SR_BSY 0x80
#define IDE_SR_DRDY 0x40
#define IDE_SR_DF 0x20
#define IDE_SR_DSC 0x10
#define IDE_SR_DRQ 0x08
#define IDE_SR_CORR 0x04
#define IDE_SR_IDX 0x02
#define IDE_SR_ERR 0x01

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

#define ATAPI_CMD_EJECT 0x1B
#define ATAPI_CMD_READ_CAPACITY 0x25
#define ATAPI_CMD_READ 0xA8

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

#define ATAPI_SECTOR_SIZE 2048

namespace DeviceManager { namespace IDE {
    DeviceDriver IDEDriver;

    uint8_t irqInvoked = 0;

    extern "C" void IRQ14Handler() { debugLogger.Log("IRQ14"); }

    extern "C" void IRQ15Handler() { irqInvoked = 1; }

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

    void ReadDataBuffer(IDEDeviceInfo* ideDevice, uint8_t channel, uint8_t* buffer, size_t length) {
        size_t quads = length / 4;
        uint8_t reg = ATA_REG_DATA;

        uint32_t* dBuffer = (uint32_t*)buffer;

        for (size_t i = 0; i < quads; i++)
            dBuffer[i] = ind(ideDevice->channels[channel].IO + reg);
    }

    bool Polling(IDEDeviceInfo* ideDevice, uint8_t channel, uint32_t advancedCheck) {
        for (int i = 0; i < 4; i++)
            ReadRegister(ideDevice, channel, ATA_REG_ALTSTATUS);

        while (ReadRegister(ideDevice, channel, ATA_REG_STATUS) & IDE_SR_BSY)
            ;

        if (advancedCheck) {
            uint8_t state = ReadRegister(ideDevice, channel, ATA_REG_STATUS);
            if (state & IDE_SR_ERR) {
                errorLogger.Log("Error while polling!");
                return false;
            }

            if (state & IDE_SR_DF) {
                errorLogger.Log("Device fault while polling!");
                return false;
            }

            if ((state & IDE_SR_DRQ) == 0) {
                errorLogger.Log("DRQ should be set");
                return false;
            }
        }

        return true;
    }

    void WaitIRQ() {
        while (!irqInvoked)
            ;
        irqInvoked = 0;
    }

    uint64_t ATAIO(IDEDriveInfo* driveInfo, IDEDeviceInfo* ideInfo, uint8_t direction, uint64_t lba, void* buffer, size_t bufferSize) {
        size_t numSectors = bufferSize / 512;

        // Disable IRQs
        WriteRegister(ideInfo, driveInfo->channel, ATA_REG_CONTROL, ideInfo->channels[driveInfo->channel].nIEN = 2);

        uint8_t lbaMode;
        uint8_t lbaIO[6];
        uint8_t head;
        uint8_t sect;
        uint16_t cyl;
        if (lba >= 0x10000000) {
            // LBA48
            lbaMode = 2;
            lbaIO[0] = lba & 0xFF;
            lbaIO[1] = (lba >> 8) & 0xFF;
            lbaIO[2] = (lba >> 16) & 0xFF;
            lbaIO[3] = (lba >> 24) & 0xFF;
            lbaIO[4] = (lba >> 32) & 0xFF;
            lbaIO[5] = (lba >> 40) & 0xFF;
            head = 0;
        } else if (driveInfo->capabilities & 0x200) {
            lbaMode = 1;
            lbaIO[0] = lba & 0xFF;
            lbaIO[1] = (lba >> 8) & 0xFF;
            lbaIO[2] = (lba >> 16) & 0xFF;
            lbaIO[3] = 0;
            lbaIO[4] = 0;
            lbaIO[5] = 0;
            head = (lba >> 24) & 0xF;
        } else {
            lbaMode = 0;
            sect = (lba % 63) + 1;
            cyl = (lba + 1 - sect) / (16 * 63);
            lbaIO[0] = sect;
            lbaIO[1] = cyl & 0xFF;
            lbaIO[2] = (cyl >> 8) & 0xFF;
            lbaIO[3] = 0;
            lbaIO[4] = 0;
            lbaIO[5] = 0;
            head = (lba + 1 - sect) % (16 * 63) / 63;
        }

        while (ReadRegister(ideInfo, driveInfo->channel, ATA_REG_STATUS) & IDE_SR_BSY)
            ;

        if (lbaMode == 0)
            WriteRegister(ideInfo, driveInfo->channel, ATA_REG_HDDEVSEL, 0xA0 | (driveInfo->drive << 4) | head);
        else
            WriteRegister(ideInfo, driveInfo->channel, ATA_REG_HDDEVSEL, 0xE0 | (driveInfo->drive << 4) | head);

        sleep(2);

        if (lbaMode == 2) {
            WriteRegister(ideInfo, driveInfo->channel, ATA_REG_SECCOUNT1, 0);
            WriteRegister(ideInfo, driveInfo->channel, ATA_REG_LBA3, lbaIO[3]);
            WriteRegister(ideInfo, driveInfo->channel, ATA_REG_LBA4, lbaIO[4]);
            WriteRegister(ideInfo, driveInfo->channel, ATA_REG_LBA5, lbaIO[5]);
        }
        WriteRegister(ideInfo, driveInfo->channel, ATA_REG_SECCOUNT0, numSectors);
        WriteRegister(ideInfo, driveInfo->channel, ATA_REG_LBA0, lbaIO[0]);
        WriteRegister(ideInfo, driveInfo->channel, ATA_REG_LBA1, lbaIO[1]);
        WriteRegister(ideInfo, driveInfo->channel, ATA_REG_LBA2, lbaIO[2]);

        uint8_t cmd = lbaMode == 2 ? ATA_CMD_READ_PIO_EXT : ATA_CMD_READ_PIO;
        WriteRegister(ideInfo, driveInfo->channel, ATA_REG_COMMAND, cmd);

        uint32_t bus = ideInfo->channels[driveInfo->channel].busMaster;
        uint32_t words = 256;
        uint64_t edi = (uint64_t)buffer;
        if (direction == 0) {
            for (size_t i = 0; i < numSectors; i++) {
                if (!Polling(ideInfo, driveInfo->channel, 1))
                    return 0;

                asm("rep insw" : : "c"(words), "d"(bus), "D"(edi));

                edi += (words * 2);
            }
        } else {
            for (size_t i = 0; i < numSectors; i++) {
                Polling(ideInfo, driveInfo->channel, 0);
                asm("rep outsw" ::"c"(words), "d"(bus), "S"(edi));
                edi += (words * 2);
            }

            WriteRegister(ideInfo, driveInfo->channel, ATA_REG_COMMAND, lbaMode == 2 ? ATA_CMD_CACHE_FLUSH_EXT : ATA_CMD_CACHE_FLUSH);
            Polling(ideInfo, driveInfo->channel, 0);
        }

        return numSectors * 512;
    }

    uint64_t ATAPIRead(IDEDriveInfo* driveInfo, IDEDeviceInfo* ideInfo, uint64_t lba, void* buffer) {
        // Setup SCSI packet
        uint8_t packet[12];
        packet[0] = ATAPI_CMD_READ;
        packet[1] = 0;
        packet[2] = (lba >> 24) & 0xFF;
        packet[3] = (lba >> 16) & 0xFF;
        packet[4] = (lba >> 8) & 0xFF;
        packet[5] = lba & 0xFF;
        packet[6] = 0;
        packet[7] = 0;
        packet[8] = 0;
        packet[9] = 1;
        packet[10] = 0;
        packet[11] = 0;

        // Send the PACKET command
        WriteRegister(ideInfo, driveInfo->channel, ATA_REG_COMMAND, ATA_CMD_PACKET);

        // Wait
        if (!Polling(ideInfo, driveInfo->channel, 1)) {
            errorLogger.Log("PACKET command error!");
            return 0;
        }

        // Send packet data
        asm("rep outsw" ::"c"(6), "d"(ideInfo->channels[driveInfo->channel].IO), "S"(packet));

        WaitIRQ();

        if (!Polling(ideInfo, driveInfo->channel, 1)) {
            errorLogger.Log("Packet error!");
            return 0;
        }

        asm("rep insw" ::"c"(ATAPI_SECTOR_SIZE / 2), "d"(ideInfo->channels[driveInfo->channel].IO), "D"(buffer));

        WaitIRQ();

        while (ReadRegister(ideInfo, driveInfo->channel, ATA_REG_STATUS) & (IDE_SR_BSY | IDE_SR_DRQ))
            ;

        return ATAPI_SECTOR_SIZE;
    }

    uint64_t IDERead(Device* device, uint64_t lba, void* buffer, size_t bufferSize) {
        // Verify device type
        if (device->driverDeviceType == (uint64_t)-1)
            return 0;

        IDEDriveInfo* driveInfo = (IDEDriveInfo*)device->driverInfo;
        IDEDeviceInfo* ideInfo = (IDEDeviceInfo*)driveInfo->device;

        if (driveInfo->type == IDE_ATA)
            return ATAIO(driveInfo, ideInfo, 0, lba, buffer, bufferSize);
        else {
            // Enable IRQs
            WriteRegister(ideInfo, driveInfo->channel, ATA_REG_CONTROL, ideInfo->channels[driveInfo->channel].nIEN = irqInvoked = 0);

            // Select drive
            WriteRegister(ideInfo, driveInfo->channel, ATA_REG_HDDEVSEL, driveInfo->drive << 4);
            sleep(2);

            // Select PIO mode
            WriteRegister(ideInfo, driveInfo->channel, ATA_REG_FEATURES, 0);

            // Set buffer size
            WriteRegister(ideInfo, driveInfo->channel, ATA_REG_LBA1, ATAPI_SECTOR_SIZE & 0xFF);
            WriteRegister(ideInfo, driveInfo->channel, ATA_REG_LBA2, ATAPI_SECTOR_SIZE >> 8);

            size_t numSectors = bufferSize / ATAPI_SECTOR_SIZE;
            uint64_t bytesRead = 0;
            uint64_t val;
            for (size_t i = 0; i < numSectors; i++) {
                val = ATAPIRead(driveInfo, ideInfo, lba, buffer);
                if (val == 0)
                    return 0;

                bytesRead += val;
                buffer = (void*)((uint64_t)buffer + ATAPI_SECTOR_SIZE);
                lba++;
            }

            return bytesRead;
        }
    }

    uint64_t IDEWrite(Device* device, uint64_t address, void* buffer, size_t bufferSize) { return 0; }

    void IDERegisterChildDriver(DeviceDriver* driver) {}

    bool IDEVerifyDevice(Device* device) {
        PCI::PCIDeviceInfo* devInfo = (PCI::PCIDeviceInfo*)device->driverInfo;

        return devInfo->classCode == 1 && devInfo->subClass == 1;
    }

    void IDERegisterDevice(Device* device) {
        PCI::StandardPCIDeviceInfo* devInfo = (PCI::StandardPCIDeviceInfo*)device->driverInfo;

        IDEDeviceInfo* ideInfo = (IDEDeviceInfo*)malloc(sizeof(IDEDeviceInfo));
        device->driverInfo = ideInfo;
        device->driverDeviceType = -1;

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

        // Install Interrupt Handlers
        InterruptHandler::SetIRQ(14, IRQ14Handler);
        InterruptHandler::SetIRQ(15, IRQ15Handler);

        free(devInfo);

        // Disable IRQs
        WriteRegister(ideInfo, ATA_PRIMARY, ATA_REG_CONTROL, 2);
        WriteRegister(ideInfo, ATA_SECONDARY, ATA_REG_CONTROL, 2);

        // Check the connected drives
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                // Select Drive
                WriteRegister(ideInfo, i, ATA_REG_HDDEVSEL, 0xA0 | (j << 4));
                sleep(2);

                // Send ATA identify command
                WriteRegister(ideInfo, i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
                sleep(2);

                // Polling
                if (ReadRegister(ideInfo, i, ATA_REG_STATUS) == 0)
                    continue;

                uint8_t err = 0;
                uint8_t type = IDE_ATA;
                uint8_t status;
                while (1) {
                    status = ReadRegister(ideInfo, i, ATA_REG_STATUS);
                    if ((status & IDE_SR_ERR)) {
                        err = 1;
                        break;
                    }

                    if (!(status & IDE_SR_BSY) && (status & IDE_SR_DRQ))
                        break;
                }

                // Probe for ATAPI
                if (err != 0) {
                    uint8_t cl = ReadRegister(ideInfo, i, ATA_REG_LBA1);
                    uint8_t ch = ReadRegister(ideInfo, i, ATA_REG_LBA2);

                    if (cl == 0x14 && ch == 0xEB)
                        type = IDE_ATAPI;
                    else if (cl == 0x69 && ch == 0x96)
                        type = IDE_ATAPI;
                    else
                        continue;

                    WriteRegister(ideInfo, i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
                    sleep(2);
                }

                // Read Identification Space
                uint8_t buffer[512];
                ReadDataBuffer(ideInfo, i, buffer, 512);

                // Read device parameters
                IDEDriveInfo* driveInfo = (IDEDriveInfo*)malloc(sizeof(IDEDriveInfo));
                driveInfo->device = ideInfo;
                driveInfo->channel = i;
                driveInfo->drive = j;
                driveInfo->type = type;
                driveInfo->sign = *((uint16_t*)(buffer + ATA_IDENT_DEVICETYPE));
                driveInfo->capabilities = *((uint16_t*)(buffer + ATA_IDENT_CAPABILITIES));
                driveInfo->commandsets = *((uint32_t*)(buffer + ATA_IDENT_COMMANDSETS));

                // Get size
                if (driveInfo->type == IDE_ATA) {
                    if (driveInfo->commandsets & (1 << 26))
                        driveInfo->size = *((uint32_t*)(buffer + ATA_IDENT_MAX_LBA_EXT));
                    else
                        driveInfo->size = *((uint32_t*)(buffer + ATA_IDENT_MAX_LBA));
                } else {
                    // Select PIO mode
                    WriteRegister(ideInfo, driveInfo->channel, ATA_REG_FEATURES, 0);

                    // Set buffer size
                    WriteRegister(ideInfo, driveInfo->channel, ATA_REG_LBA1, 0x08);
                    WriteRegister(ideInfo, driveInfo->channel, ATA_REG_LBA2, 0x00);

                    // Setup SCSI packet
                    uint8_t packet[12];
                    packet[0] = ATAPI_CMD_READ_CAPACITY;
                    packet[1] = 0;
                    packet[2] = 0;
                    packet[3] = 0;
                    packet[4] = 0;
                    packet[5] = 0;
                    packet[6] = 0;
                    packet[7] = 0;
                    packet[8] = 0;
                    packet[9] = 0;
                    packet[10] = 0;
                    packet[11] = 0;

                    // Send the PACKET command
                    WriteRegister(ideInfo, driveInfo->channel, ATA_REG_COMMAND, ATA_CMD_PACKET);

                    // Wait
                    if (!Polling(ideInfo, driveInfo->channel, 1)) {
                        errorLogger.Log("PACKET command error!");
                        driveInfo->size = 0;
                    } else {
                        // Send packet data
                        asm("rep outsw" ::"c"(6), "d"(ideInfo->channels[driveInfo->channel].IO), "S"(packet));

                        // Wait
                        if (!Polling(ideInfo, driveInfo->channel, 1)) {
                            errorLogger.Log("PACKET command error!");
                            driveInfo->size = 0;
                        } else {
                            uint32_t lba = (uint32_t)inw(ideInfo->channels[driveInfo->channel].IO) << 16;
                            lba |= inw(ideInfo->channels[driveInfo->channel].IO);
                            uint32_t blockSize = (uint32_t)inw(ideInfo->channels[driveInfo->channel].IO) << 16;
                            blockSize |= inw(ideInfo->channels[driveInfo->channel].IO);

                            lba++;

                            driveInfo->size = lba * ATAPI_SECTOR_SIZE;
                        }
                    }
                }

                // Get model
                for (int k = 0; k < 40; k += 2) {
                    driveInfo->model[k] = buffer[ATA_IDENT_MODEL + k + 1];
                    driveInfo->model[k + 1] = buffer[ATA_IDENT_MODEL + k];
                }

                driveInfo->model[40] = 0;

                Device* newDrive = (Device*)malloc(sizeof(Device));
                newDrive->name = (char*)driveInfo->model;
                newDrive->driver = &IDEDriver;
                newDrive->driverDeviceType = driveInfo->type;
                newDrive->driverInfo = driveInfo;
                newDrive->next = IDEDriver.deviceHead;
                newDrive->prev = nullptr;

                if (IDEDriver.deviceHead != nullptr)
                    IDEDriver.deviceHead->prev = newDrive;

                IDEDriver.deviceHead = newDrive;

                infoLogger.Log("%s Drive at %s %s with size %i bytes - %s", (const char*[]){"ATA", "ATAPI"}[driveInfo->type], (const char*[]){"Primary", "Secondary"}[driveInfo->channel], (const char*[]){"Master", "Slave"}[driveInfo->drive], driveInfo->size, driveInfo->model);

                VirtualFileSystem::RegisterDrive(newDrive, driveInfo->size);
            }
        }
    } // namespace IDE

    void RegisterIDEDriver() {
        infoLogger.Log("Initializing IDE . . . ");

        IDEDriver.RegisterChildDriver = IDERegisterChildDriver;
        IDEDriver.signature = DEVICE_DRIVER_SIGNATURE_IDE;
        IDEDriver.name = "LOS IDE Driver";
        IDEDriver.parent = GetDeviceDriver(DEVICE_DRIVER_SIGNATURE_PCI);
        IDEDriver.VerifyDevice = IDEVerifyDevice;
        IDEDriver.RegisterDevice = IDERegisterDevice;
        IDEDriver.ReadStream = IDERead;
        RegisterDeviceDriver(&IDEDriver);

        infoLogger.Log("IDE Initialized!");
    }
}} // namespace DeviceManager::IDE
