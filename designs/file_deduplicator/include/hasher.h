#pragma once
#include <string>
#include <memory>
#include "config.h"

namespace bayan {
    /// Abstract base for a block hash function.
    class Hasher {
    public:
        virtual ~Hasher() = default;

        /// Feed raw bytes (may be less than the block size for the last padded block).
        virtual void update(const void *data, std::size_t size) = 0;

        /// Return the hash of the data fed so far as a printable string.
        [[nodiscard]] virtual std::string digest() const = 0;

        /// Reset the internal state so the same object can be reused for another block.
        virtual void reset() = 0;
    };

    /// Factory – creates a concrete Hasher according to the chosen algorithm.
    std::unique_ptr<Hasher> make_hasher(HashAlgo algo);
}
