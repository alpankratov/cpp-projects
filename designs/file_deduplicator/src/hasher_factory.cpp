#include "../include/hasher.h"
#include "../include/crc32_hasher.h"
#include "../include/md5_hasher.h"

namespace bayan {
    std::unique_ptr<Hasher> make_hasher(const HashAlgo algo) {
        switch (algo) {
            case HashAlgo::CRC32:
                return std::make_unique<CRC32Hasher>();
            case HashAlgo::MD5:
                return std::make_unique<MD5Hasher>();
            default:
                // Fallback to CRC32 to be safe; should not happen.
                return std::make_unique<CRC32Hasher>();
        }
    }
} // namespace bayan
