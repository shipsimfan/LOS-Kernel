#include <fs.h>

#include <logger.h>
#include <stdlib.h>
#include <string.h>

namespace VirtualFileSystem {
    namespace ISO9660 {
        bool VerifyISO9660(FileSystem* fs) {
            // Look for 'CD001' signature of ISO9660 volume descriptor
            uint8_t* sector = (uint8_t*)malloc(2048);
            uint64_t bytesRead = fs->device->driver->ReadStream(fs->device, 0x10, sector, 2048);
            if (bytesRead == 0) {
                errorLogger.Log("Error while reading sector!");
                return false;
            }

            if (memcmp(sector + 1, "CD001", 5) == 0)
                return true;

            return false;
        }

        void InitISO9660(FileSystem* fs) { debugLogger.Log("Initializing ISO 9660 File System"); }
    } // namespace ISO9660

    FileSystemDriver ISO9660Driver = {"ISO9660", nullptr, ISO9660::VerifyISO9660, ISO9660::InitISO9660}; // namespace VirtualFileSystem
} // namespace VirtualFileSystem