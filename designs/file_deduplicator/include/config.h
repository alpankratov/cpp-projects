#pragma once
#include <boost/filesystem.hpp>
#include <string>
#include <vector>

namespace bayan {

    enum class HashAlgo { CRC32, MD5 };

    struct Config
    {
        std::vector<boost::filesystem::path> scan_dirs;
        std::vector<boost::filesystem::path> exclude_dirs;
        int depth = -1;                     // -1 → unlimited recursion
        std::uintmax_t min_size = 2;        // > 1 byte by default
        std::vector<std::string> masks;    // case‑insensitive glob patterns
        std::size_t block_size = 4096;      // default block size
        HashAlgo hash_algo = HashAlgo::CRC32;
    };

    /// Parses command line arguments with Boost.Program_options and fills a Config.
    Config parse_config(int argc, char* argv[]);
}