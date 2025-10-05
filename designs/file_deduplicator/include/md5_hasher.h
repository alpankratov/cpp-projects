#pragma once
#include "hasher.h"
#include <boost/uuid/detail/md5.hpp>
#include <iomanip>
#include <sstream>

namespace bayan {

    class MD5Hasher : public Hasher
    {
    public:
        MD5Hasher() { reset(); }

        void update(const void* data, std::size_t size) override
        {
            md5_process(data, size);
        }

        std::string digest() const override
        {
            std::ostringstream oss;
            const auto& d = result;
            for (int i = 0; i < 16; ++i)
                oss << std::hex << std::setw(2) << std::setfill('0')
                    << static_cast<int>(static_cast<unsigned char>(d[i]));
            return oss.str();
        }

        void reset() override
        {
            ctx = boost::uuids::detail::md5();
            result.clear();
        }

    private:
        void md5_process(const void* data, std::size_t size)
        {
            ctx.process_bytes(data, size);
            if (size == 0)   // finalisation request
            {
                boost::uuids::detail::md5::digest_type d;
                ctx.get_digest(d);
                result.assign(reinterpret_cast<const char*>(d), 16);
            }
        }

        boost::uuids::detail::md5 ctx;
        std::string result;          // 16‑byte binary digest
    };

}