#pragma once
#include <vector>
#include <optional>
#include <algorithm>
#include <iostream>
#include <memory>
#include "shape.h"
#include "shape_id.h"

using namespace shape_id;
using namespace shape;

namespace document {
    class Document {
    public:
        using ShapePtr = std::shared_ptr<IShape>;

        ShapeId add_shape(ShapePtr s) {
            auto id = ids_.next();
            std::cout << "\n---------------- Removing new shape of type: " << s->type() << " with id " <<
                                id.value << "----------------\n";
            shapes_.push_back({id, std::move(s)});
            return id;
        }

        bool remove_shape(ShapeId id) {
            std::cout << "\n---------------- Removing shape id: " << id.value << " ----------------\n";
            auto it = std::remove_if(shapes_.begin(), shapes_.end(),
                                     [&](const Entry &e) { return e.id == id; });
            const bool removed = (it != shapes_.end());
            shapes_.erase(it, shapes_.end());
            return removed;
        }

        std::optional<ShapePtr> find(ShapeId id) const {
            std::cout << "\n---------------- Looking for shape id:" << id.value << " ----------------\n";
            for (auto &e: shapes_) if (e.id == id) return e.shape;
            return std::nullopt;
        }

        const std::vector<ShapePtr> list() const {
            std::cout << "\n---------------- Listing shapes ----------------\n";
            std::vector<ShapePtr> out;
            out.reserve(shapes_.size());
            for (auto &e: shapes_) {
                e.shape->explain();
                out.push_back(e.shape);
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
