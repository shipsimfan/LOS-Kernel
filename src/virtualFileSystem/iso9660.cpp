#include <fs.h>

namespace VirtualFileSystem {
    namespace ISO9660 {
        bool VerifyISO9660(FileSystem* fs) { return false; }

        void InitISO9660(FileSystem* fs) {}
    } // namespace ISO9660

    FileSystemDriver ISO9660Driver = {"ISO9660", nullptr, ISO9660::VerifyISO9660, ISO9660::InitISO9660}; // namespace VirtualFileSystem
} // namespace VirtualFileSystem