/*
* A trivial ID generator. In a real project you'd likely centralize UUIDs.
 * Here we keep it simple and deterministic for the demo.
 */
#pragma once

namespace shape_id {
    struct ShapeId {
        std::uint64_t value{};
        friend bool operator==(const ShapeId a, const ShapeId b) noexcept { return a.value == b.value; }
        friend bool operator!=(const ShapeId a, const ShapeId b) noexcept { return !(a == b); }
    };

    class IdGenerator {
    public:
        ShapeId next() noexcept { return ShapeId{++counter_}; }

    private:
        std::uint64_t counter_{0};
    };
}
