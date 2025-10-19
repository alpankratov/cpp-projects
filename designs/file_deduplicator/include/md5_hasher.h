#pragma once
#include "hasher.h"
#include <boost/uuid/detail/md5.hpp>
#include <iomanip>
#include <sstream>

namespace bayan {
    class MD5Hasher final : public Hasher {
    public:
        MD5Hasher() { reset(); }

        void update(const void *data, const std::size_t size) override {
            ctx.process_bytes(data, size);
        }

        [[nodiscard]] std::string digest() const override {
            // Compute digest from a copy of the current context so that
            // calling digest() does not mutate the hasher state.
            auto tmp = ctx; // copy current state
            boost::uuids::detail::md5::digest_type d;
            tmp.get_digest(d);

            const auto bytes = reinterpret_cast<const unsigned char *>(&d);
            std::ostringstream oss;
            oss << std::hex << std::setfill('0');
            for (int i = 0; i < 16; ++i)
                oss << std::setw(2) << static_cast<unsigned int>(bytes[i]);
            return oss.str();
        }

        void reset() override {
            ctx = boost::uuids::detail::md5();
        }

    private:
        boost::uuids::detail::md5 ctx;
    };
}
