#include "../include/block_reader.h"
#include <algorithm>

namespace bayan {

    BlockReader::BlockReader(const boost::filesystem::path& p,
                             std::size_t block_sz)
        : block_size_(block_sz)
    {
        stream_.open(p.string(), std::ios::binary);
        if (!stream_)
            throw std::runtime_error("Cannot open file: " + p.string());
    }

    std::vector<unsigned char> BlockReader::next()
    {
        std::vector<unsigned char> buf(block_size_, 0);
        if (eof_) return buf;               // already past EOF – return zeroed block

        stream_.read(reinterpret_cast<char*>(buf.data()), block_size_);
        std::streamsize got = stream_.gcount();

        if (got < static_cast<std::streamsize>(block_size_))
        {
            // Zero‑pad the remainder (already zero‑filled by constructor)
            eof_ = true;
        }
        return buf;
    }
}