#include <fs.h>

#include <fs/iso9660.h>
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
                free(sector);
                return false;
            }

            if (memcmp(sector + 1, "CD001", 5) == 0) {
                free(sector);
                return true;
            }

            free(sector);
            return false;
        }

        void SetupDirectory(FileSystem* fs, Directory* directory, void* bufferStart, size_t bufferLength) {
            DirectoryEntry* entry = (DirectoryEntry*)bufferStart;
            uint64_t entryI = (uint64_t)entry;

            // Skip first two entries (self & parent)
            entryI += entry->length;
            entry = (DirectoryEntry*)entryI;
            entryI += entry->length;
            entry = (DirectoryEntry*)entryI;

            // Read the entries
            while (entry->length > 0) {
                if (entry->flags & 2) {
                    Directory* newDir = (Directory*)malloc(sizeof(Directory));

                    newDir->next = directory->subDirectories;
                    directory->subDirectories = newDir;
                    newDir->parent = directory;

                    newDir->subDirectories = nullptr;
                    newDir->subFiles = nullptr;

                    char* name = (char*)malloc(entry->filenameLength + 1);
                    memcpy(name, &entry->fileIdentifier, entry->filenameLength);
                    name[entry->filenameLength] = 0;
                    newDir->name = name;

                    newDir->fileSystem = directory->fileSystem;

                    void* buffer = malloc(entry->dirLength);
                    uint64_t bytesRead = fs->device->driver->ReadStream(fs->device, entry->lba, buffer, entry->dirLength);
                    if (bytesRead == 0) {
                        errorLogger.Log("Error while reading sectors!");
                        return;
                    }

                    SetupDirectory(fs, newDir, buffer, entry->dirLength);

                    free(buffer);
                } else {
                    File* newFile = (File*)malloc(sizeof(File));
                    newFile->next = directory->subFiles;
                    directory->subFiles = newFile;
                    newFile->directory = directory;

                    newFile->fileSystem = directory->fileSystem;

                    // Find file name length
                    uint64_t nameLength = 0;
                    char* fileIdentifier = (char*)(&entry->fileIdentifier);
                    for (; fileIdentifier[nameLength] != '.'; nameLength++)
                        ;

                    char* filename = (char*)malloc(nameLength + 1);
                    memcpy(filename, fileIdentifier, nameLength);
                    filename[nameLength] = 0;
                    newFile->name = filename;

                    // Find file extension length
                    nameLength++;
                    uint64_t extensionLength = 0;
                    for (; fileIdentifier[nameLength + extensionLength] != ';'; extensionLength++)
                        ;

                    char* extension = (char*)malloc(extensionLength + 1);
                    memcpy(extension, fileIdentifier + nameLength, extensionLength);
                    newFile->extension = extension;

                    newFile->fileSize = entry->dirLength;

                    newFile->driverInfo = (void*)entry->lba;
                }

                entryI += entry->length;
                entry = (DirectoryEntry*)entryI;
            }
        }

        void InitISO9660(FileSystem* fs) {
            // Locate Primary Volume Descriptor
            uint64_t lba = 0x10;
            uint8_t* sector = (uint8_t*)malloc(2048);
            while (1) {
                // Load the descriptor sector
                uint64_t bytesRead = fs->device->driver->ReadStream(fs->device, lba, sector, 2048);
                if (bytesRead == 0) {
                    errorLogger.Log("Error while reading sector!");
                    free(sector);
                    return;
                }

                // Verify Identifier
                if (memcmp(sector + 1, "CD001", 5)) {
                    errorLogger.Log("Invalid sector identifier!");
                    free(sector);
                    return;
                }

                // Check descriptor type
                if (sector[0] == 1)
                    break;

                if (sector[0] == 0xFF) {
                    errorLogger.Log("No primary volume descriptor located!");
                    free(sector);
                    return;
                }

                lba++;
            }

            // sector now holds the primary volume descriptor
            char* volumeName = (char*)malloc(sizeof(char) * 32);
            strncpy(volumeName, (const char*)(sector + 40), 32);

            fs->volumeName = volumeName;

            Directory* rootDir = (Directory*)malloc(sizeof(Directory));
            rootDir->name = "";
            rootDir->parent = rootDir;
            rootDir->next = nullptr;
            rootDir->subDirectories = nullptr;
            rootDir->subFiles = nullptr;
            rootDir->fileSystem = fs;

            DirectoryEntry* entry = (DirectoryEntry*)(sector + 156);
            uint32_t rootLBA = entry->lba;
            uint32_t rootLength = entry->dirLength;

            free(sector);

            void* rootDirectory = malloc(rootLength);
            uint64_t bytesRead = fs->device->driver->ReadStream(fs->device, rootLBA, rootDirectory, rootLength);
            if (bytesRead == 0) {
                errorLogger.Log("Error while reading sectors!");
                free(rootDirectory);
                return;
            }

            SetupDirectory(fs, rootDir, rootDirectory, rootLength);

            fs->rootDir = rootDir;

            free(rootDirectory);
        }

        uint64_t ReadFile(File* file, size_t offset, void* buffer, size_t bufferSize) {
            void* bufToUse = buffer;
            size_t bufToUseSize = bufferSize;
            if (bufferSize % 2048 != 0) {
                bufToUseSize = ((bufferSize / 2048) + 1) * 2048;
                bufToUse = malloc(bufToUseSize);
            }

            debugLogger.Log("Loading %i bytes from %i", bufToUseSize, file->driverInfo);
            file->fileSystem->device->driver->ReadStream(file->fileSystem->device, (uint64_t)file->driverInfo, bufToUse, bufToUseSize);

            if (buffer != bufToUse) {
                memcpy(buffer, bufToUse, bufferSize);
                free(bufToUse);
            }

            return bufferSize;
        }
    } // namespace ISO9660

    FileSystemDriver ISO9660Driver = {"ISO9660", nullptr, ISO9660::VerifyISO9660, ISO9660::InitISO9660, ISO9660::ReadFile}; // namespace VirtualFileSystem
} // namespace VirtualFileSystem