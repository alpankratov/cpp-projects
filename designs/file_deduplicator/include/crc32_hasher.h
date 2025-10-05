#pragma once
#include "hasher.h"
#include <boost/crc.hpp>
#include <iomanip>
#include <sstream>

namespace bayan {

    class CRC32Hasher : public Hasher
    {
    public:
        CRC32Hasher() { reset(); }

        void update(const void* data, std::size_t size) override
        {
            crc.process_bytes(data, size);
        }

        std::string digest() const override
        {
            std::ostringstream oss;
            oss << std::hex << std::setw(8) << std::setfill('0')
                << static_cast<std::uint32_t>(crc.checksum());
            return oss.str();
        }

        void reset() override { crc.reset(); }

    private:
        boost::crc_32_type crc;
    };

}