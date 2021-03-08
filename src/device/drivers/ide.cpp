#include <device/drivers/ide.h>

#include "ide.h"
#include "pci.h"

#include <console.h>
#include <device/drivers/pci.h>
#include <device/manager.h>
#include <device/util.h>
#include <errno.h>
#include <filesystem/driver.h>
#include <interrupt/irq.h>
#include <string.h>
#include <time.h>

void InitializeIDEDriver() {
    Queue<Device::Device> queue;
    Device::GetDevices(Device::Device::Type::PCI_DEVICE, queue);

    if (queue.front() != nullptr) {
        Queue<Device::Device>::Iterator iter(&queue);
        do {
            uint64_t devClass, subClass;
            iter.value->Open();
            if (iter.value->Read(PCI_CONFIG_CLASS, &devClass) != SUCCESS) {
                iter.value->Close();
                Console::SetForegroundColor(0xFF, 0x00, 0x00);
                Console::Println("Failed to read PCI device class!");
                Console::SetForegroundColor(0xFF, 0xFF, 0xFF);
                continue;
            }

            if (iter.value->Read(PCI_CONFIG_SUB_CLASS, &subClass) != SUCCESS) {
                iter.value->Close();
                Console::SetForegroundColor(0xFF, 0x00, 0x00);
                Console::Println("Failed to read PCI device sub class!");
                Console::SetForegroundColor(0xFF, 0xFF, 0xFF);
                continue;
            }

            iter.value->Close();

            if (devClass == 1 && subClass == 1) {
                PCIDevice* oldDevice = (PCIDevice*)iter.value;
                IDEDevice* newDevice = new IDEDevice(oldDevice);
                Device::RegisterDevice(nullptr, newDevice);
                iter.Remove();
                delete oldDevice;
                return;
            }
        } while (iter.Next());
    }
}

IDEDevice::IDEDevice(PCIDevice* pciDevice) : Device("IDE Controller", Type::CONTROLLER), irq(false) {
    // Save the address
    uint64_t value;
    pciDevice->Open();
    if (pciDevice->Read(PCI_CONFIG_BAR_0, &value) != SUCCESS)
        return;
    channels[ATA_PRIMARY].IO = value;

    if (pciDevice->Read(PCI_CONFIG_BAR_1, &value) != SUCCESS)
        return;
    channels[ATA_PRIMARY].control = value;

    if (pciDevice->Read(PCI_CONFIG_BAR_2, &value) != SUCCESS)
        return;
    channels[ATA_SECONDARY].IO = value;

    if (pciDevice->Read(PCI_CONFIG_BAR_3, &value) != SUCCESS)
        return;
    channels[ATA_SECONDARY].control = value;

    if (pciDevice->Read(PCI_CONFIG_BAR_4, &value) != SUCCESS)
        return;
    channels[ATA_PRIMARY].busMaster = value;
    channels[ATA_SECONDARY].busMaster = value + 8;
    pciDevice->Close();

    // Correct address
    if (channels[ATA_PRIMARY].IO <= 1)
        channels[ATA_PRIMARY].IO = 0x1F0;

    if (channels[ATA_PRIMARY].control <= 1)
        channels[ATA_PRIMARY].control = 0x3F4;

    if (channels[ATA_SECONDARY].IO <= 1)
        channels[ATA_SECONDARY].IO = 0x170;

    if (channels[ATA_SECONDARY].control <= 1)
        channels[ATA_SECONDARY].control = 0x374;

    // Install IRQ Handlers
    Interrupt::InstallIRQHandler(14, IRQHandler, this);
    Interrupt::InstallIRQHandler(15, IRQHandler, this);

    this->Open();

    // Disable IRQs
    if (Write(ATA_PRIMARY_REG(ATA_REG_CONTROL), 2) != SUCCESS)
        return;

    if (Write(ATA_SECONDARY_REG(ATA_REG_CONTROL), 2) != SUCCESS)
        return;

    // Check for connected drives
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            channels[i].drives[j] = nullptr;

            // Select drive
            if (Write((i << 8) | ATA_REG_HDDEVSEL, 0xA0 | (j << 4)) != SUCCESS)
                continue;
            Sleep(2);

            if (Write((i << 8) | ATA_REG_COMMAND, ATA_CMD_IDENTIFY) != SUCCESS)
                continue;
            Sleep(2);

            if (Read((i << 8) | ATA_REG_STATUS, &value) != SUCCESS)
                continue;
            if (value == 0)
                continue;

            uint8_t err = 0;
            while (1) {
                if (Read((i << 8) | ATA_REG_STATUS, &value) != SUCCESS) {
                    err = 2;
                    break;
                }

                if (value & ATA_SR_ERR) {
                    err = 1;
                    break;
                }

                if (!(value & ATA_SR_BSY) && (value & ATA_SR_DRQ))
                    break;
            }

            // Probe for ATAPI
            uint8_t type = IDE_ATA;

            if (err != 0) {
                if (Read((i << 8) | ATA_REG_LBA1, &value) != SUCCESS)
                    continue;

                uint8_t cl = value;

                if (Read((i << 8) | ATA_REG_LBA2, &value) != SUCCESS)
                    continue;

                uint8_t ch = value;

                if (cl == 0x14 && ch == 0xEB)
                    type = IDE_ATAPI;
                else if (cl == 0x69 && ch == 0x96)
                    type = IDE_ATAPI;
                else
                    continue;

                if (Write((i << 8) | ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET) != SUCCESS)
                    continue;
                Sleep(2);
            }

            if (type == IDE_ATA)
                Console::Println("[ IDE ] Found ATA drive. Only ATAPI is currently supported.");
            else {
                this->Close();
                channels[i].drives[j] = new ATAPIDevice(this, i, j);
                RegisterDevice(this, channels[i].drives[j]);
                this->Open();
            }
        }
    }

    this->Close();
}

uint64_t IDEDevice::OnOpen() { return SUCCESS; }
uint64_t IDEDevice::OnClose() { return SUCCESS; }

uint64_t IDEDevice::DoRead(uint64_t address, uint64_t* value) {
    uint8_t reg = address & 0xFF;
    uint8_t channel = (address >> 8) & 1;

    if (reg > 0x07 && reg < 0x0C) {
        uint64_t status = Write((address & 0x100) | ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
        if (status != SUCCESS)
            return status;
    }

    if (reg < 0x08)
        *value = inb(channels[channel].IO + reg);
    else if (reg < 0x0C)
        *value = inb(channels[channel].IO + reg - 0x06);
    else if (reg < 0x0E)
        *value = inb(channels[channel].control + reg - 0x0A);
    else if (reg < 0x16)
        *value = inb(channels[channel].busMaster + reg - 0x0E);

    if (reg > 0x07 && reg < 0x0C) {
        uint64_t status = Write((address & 0x100) | ATA_REG_CONTROL, channels[channel].nIEN);
        if (status != SUCCESS)
            return status;
    }

    return SUCCESS;
}

uint64_t IDEDevice::DoReadStream(uint64_t address, void* buffer, int64_t count, int64_t& countRead) {
    if (count < 0)
        return ERROR_BAD_PARAMETER;

    int64_t quads = count / 4;
    uint32_t* dBuffer = (uint32_t*)buffer;

    for (int64_t i = 0; i < quads; i++)
        dBuffer[i] = ind(channels[address].IO + ATA_REG_DATA);

    countRead = quads * 4;
    return SUCCESS;
}

uint64_t IDEDevice::DoWrite(uint64_t address, uint64_t value) {
    uint8_t reg = address & 0xFF;
    uint8_t channel = (address >> 8) & 1;

    if (reg > 0x07 && reg < 0x0C) {
        uint64_t status = Write((address & 0x100) | ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
        if (status != SUCCESS)
            return status;
    }

    if (reg < 0x08)
        outb(channels[channel].IO + reg, value);
    else if (reg < 0x0C)
        outb(channels[channel].IO + reg - 0x06, value);
    else if (reg < 0x0E)
        outb(channels[channel].control + reg - 0x0A, value);
    else if (reg < 0x16)
        outb(channels[channel].busMaster + reg - 0x0E, value);

    if (reg > 0x07 && reg < 0x0C) {
        uint64_t status = Write((address & 0x100) | ATA_REG_CONTROL, channels[channel].nIEN);
        if (status != SUCCESS)
            return status;
    }

    return SUCCESS;
}

uint64_t IDEDevice::DoWriteStream(uint64_t address, void* buffer, int64_t count, int64_t& countWritten) { return ERROR_NOT_IMPLEMENTED; }

void IDEDevice::WaitIRQ() {
    while (!irq)
        ;
    irq = false;
}

void IDEDevice::IRQHandler(void* context) {
    IDEDevice* ide = (IDEDevice*)context;
    ide->irq = true;
}

ATAPIDevice::ATAPIDevice(IDEDevice* ide, uint8_t channel, uint8_t drive) : Device("", Type::CD_DRIVE), ide(ide), channel(channel), drive(drive) {
    this->Open();
    ide->Open();

    // Read Identification Space
    uint8_t buffer[512];
    if (ide->ReadStream(channel, buffer, 512) < 0)
        return;

    // Read device parameters
    sign = *((uint16_t*)(buffer + ATA_IDENT_DEVICETYPE));
    capabilities = *((uint16_t*)(buffer + ATA_IDENT_CAPABILITIES));
    commandSets = *((uint32_t*)(buffer + ATA_IDENT_COMMANDSETS));

    // Get Size
    // Select PIO Mode
    if (Write(ATA_REG_FEATURES, 0) != SUCCESS)
        return;

    // Set buffer size
    if (Write(ATA_REG_LBA1, 0x08) != SUCCESS)
        return;

    if (Write(ATA_REG_LBA2, 0x00) != SUCCESS)
        return;

    // Setup SCSI packet
    uint8_t packet[12];
    memset(packet, 0, sizeof(packet));
    packet[0] = ATAPI_CMD_READ_CAPACITY;

    // Send PACKET command
    Write(ATA_REG_COMMAND, ATA_CMD_PACKET);

    // Wait
    if (!Polling(true))
        return;

    asm("rep outsw" ::"c"(6), "d"(ide->channels[channel].IO), "S"(packet));

    if (!Polling(true))
        return;

    uint32_t lba = (uint32_t)inw(ide->channels[channel].IO) << 16;
    lba |= inw(ide->channels[channel].IO);
    uint32_t blockSize = (uint32_t)inw(ide->channels[channel].IO) << 16;
    blockSize |= inw(ide->channels[channel].IO);

    ide->Close();

    lba++;

    size = lba * ATAPI_SECTOR_SIZE;

    // Get model
    char* newName = new char[41];
    for (int i = 0; i < 40; i += 2) {
        newName[i] = buffer[ATA_IDENT_MODEL + i + 1];
        newName[i + 1] = buffer[ATA_IDENT_MODEL + i];
    }

    newName[40] = 0;
    SetName(newName);
    delete newName;

    this->Close();

    // Register drive
    Console::Println("[ IDE ] New ATAPI Drive (%i MB) - %s", size / 1024 / 1024, GetName());
    RegisterDrive(this, size);
}

uint64_t ATAPIDevice::OnOpen() { return SUCCESS; }
uint64_t ATAPIDevice::OnClose() { return SUCCESS; }

uint64_t ATAPIDevice::DoRead(uint64_t address, uint64_t* value) { return ide->Read((channel << 8) | address, value); }
uint64_t ATAPIDevice::DoWrite(uint64_t address, uint64_t value) { return ide->Write((channel << 8) | address, value); }

uint64_t ATAPIDevice::DoReadStream(uint64_t address, void* buffer, int64_t count, int64_t& countRead) {
    if (count < 0)
        return ERROR_BAD_PARAMETER;

    if (count > size / ATAPI_SECTOR_SIZE)
        return ERROR_OUT_OF_RANGE;

    // Lock the IDE Device
    ide->Open();

    // Select Drive
    uint64_t status = Write(ATA_REG_HDDEVSEL, drive << 4);
    if (status != SUCCESS) {
        ide->Close();
        return status;
    }
    Sleep(2);

    // Enable IRQs
    status = Write(ATA_REG_CONTROL, ide->channels[channel].nIEN = 0);
    if (status != SUCCESS) {
        ide->Close();
        return status;
    }

    // Select PIO Mode
    status = Write(ATA_REG_FEATURES, 0);
    if (status != SUCCESS) {
        ide->Close();
        return status;
    }

    // Set buffer size
    status = Write(ATA_REG_LBA1, ATAPI_SECTOR_SIZE & 0xFF);
    if (status != SUCCESS) {
        ide->Close();
        return status;
    }

    status = Write(ATA_REG_LBA2, ATAPI_SECTOR_SIZE >> 8);
    if (status != SUCCESS) {
        ide->Close();
        return status;
    }

    // Prepare the packet
    uint8_t packet[12];
    memset(packet, 0, sizeof(packet));
    packet[0] = ATAPI_CMD_READ;
    packet[9] = 1;

    // Read the data
    int64_t numSectors = count / ATAPI_SECTOR_SIZE;
    countRead = 0;
    uint64_t value;
    uint32_t lba = address;
    uint16_t* wBuffer = (uint16_t*)buffer;
    for (int64_t i = 0; i < numSectors; i++) {
        // Insert lba into packet
        packet[2] = (lba >> 24) & 0xFF;
        packet[3] = (lba >> 16) & 0xFF;
        packet[4] = (lba >> 8) & 0xFF;
        packet[5] = lba & 0xFF;

        // Send PACKET command
        status = Write(ATA_REG_COMMAND, ATA_CMD_PACKET);
        if (status != SUCCESS) {
            ide->Close();
            return status;
        }

        // Wait
        if (!Polling(true)) {
            ide->Close();
            return ERROR_DEVICE_ERROR;
        }

        // Send packet
        asm("rep outsw" ::"c"(6), "d"(ide->channels[channel].IO), "S"(packet));
        ide->WaitIRQ();

        if (!Polling(true)) {
            ide->Close();
            return ERROR_DEVICE_ERROR;
        }

        // Read Data
        for (int i = 0; i < ATAPI_SECTOR_SIZE / 2; i++)
            wBuffer[i] = inw(ide->channels[channel].IO + ATA_REG_DATA);

        ide->WaitIRQ();

        do {
            status = Read(ATA_REG_STATUS, &value);
            if (status != SUCCESS) {
                ide->Close();
                return status;
            }
        } while (value & (ATA_SR_BSY | ATA_SR_DRQ));

        countRead += ATAPI_SECTOR_SIZE;
        wBuffer = (uint16_t*)((uint64_t)wBuffer + ATAPI_SECTOR_SIZE);
        lba++;
    }

    ide->Close();

    return SUCCESS;
}

uint64_t ATAPIDevice::DoWriteStream(uint64_t address, void* buffer, int64_t count, int64_t& countWritten) { return ERROR_NOT_IMPLEMENTED; }

bool ATAPIDevice::Polling(bool advancedCheck) {
    uint64_t state;
    for (int i = 0; i < 4; i++)
        if (Read(ATA_REG_ALTSTATUS, &state) != SUCCESS)
            return false;

    do
        if (Read(ATA_REG_STATUS, &state) != SUCCESS)
            return false;
    while (state & ATA_SR_BSY);

    if (advancedCheck) {
        if (Read(ATA_REG_STATUS, &state) != SUCCESS)
            return false;

        if (state & ATA_SR_ERR)
            return false;

        if (state & ATA_SR_DF)
            return false;

        if ((state & ATA_SR_DRQ) == 0)
            return false;
    }

    return true;
}