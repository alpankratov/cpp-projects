#pragma once
#include <vector>
#include <optional>
#include <algorithm>
#include <iostream>
#include <memory>
#include <unordered_map>

#include "shape.h"
#include "shape_id.h"

using namespace shape_id;
using namespace shape;

namespace document {
    class Document {
    public:
        using ShapePtr = std::shared_ptr<IShape>;

        ShapeId add_shape(ShapePtr s) {
            const auto id = ids_.next();
            std::cout << "\n---------------- Adding a new shape of type: " << s->type() << " with id " <<
                    id.value << "----------------\n";
            shapes_.push_back({id, std::move(s)});
            return id;
        }

        bool remove_shape(ShapeId id) {
            std::cout << "\n---------------- Removing shape id: " << id.value << " ----------------\n";
            const auto it = std::ranges::remove_if(shapes_,
                                                   [&](const Entry &e) { return e.id == id; }).begin();
            const bool removed = (it != shapes_.end());
            shapes_.erase(it, shapes_.end());
            return removed;
        }

        [[nodiscard]] std::optional<ShapePtr> find(const ShapeId id) const {
            std::cout << "\n---------------- Looking for shape id:" << id.value << " ----------------\n";
            for (const auto &[shape_id, shape]: shapes_) if (shape_id == id) return shape;
            return std::nullopt;
        }

        [[nodiscard]] std::unordered_map<int, ShapePtr> parse_into_map() const {
            std::unordered_map<int, ShapePtr> out;
            out.reserve(shapes_.size());
            for (const auto &[shape_id, shape]: shapes_) {
                out.emplace(shape_id.value, shape);
            };
            return out;
        }

        void clear() {
            std::cout << "\n---------------- Clearing document ----------------\n";
            shapes_.clear();
        }

    private:
        struct Entry {
            ShapeId id{};
            ShapePtr shape;
        };

        std::vector<Entry> shapes_{};
        IdGenerator ids_{};
    };
}
