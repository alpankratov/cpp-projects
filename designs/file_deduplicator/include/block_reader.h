#pragma once
#include <boost/filesystem.hpp>
#include <fstream>
#include <vector>

namespace bayan {
    /**
     * Reads a file block‑wise, guaranteeing:
     *   • each block is read at most once,
     *   • the last block is zero‑padded to the configured size.
     */
    class BlockReader {
    public:
        explicit BlockReader(const boost::filesystem::path &p, std::size_t block_sz);

        /** Returns true if there is another block to read. */
        bool has_next() const { return !eof_; }

        /** Reads the next block (zero‑padded if needed) and returns it. */
        std::vector<unsigned char> next();

    private:
        std::ifstream stream_;
        std::size_t block_size_;
        bool eof_ = false;
    };
}
