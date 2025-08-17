#pragma once
#include <functional>
#include <memory>
#include <string>

#include "shape.h"

using namespace shape;

class ShapeFactory {
public:
    using Creator = std::function<std::shared_ptr<IShape>()>;
    void register_type(std::string type, Creator c) { creators_[std::move(type)] = std::move(c); }

    std::shared_ptr<IShape> create(const std::string &type) const {
        if (const auto it = creators_.find(type); it != creators_.end()) {
            return it->second();
        }
        throw std::invalid_argument("Unknown shape type: " + type);
    }

private:
    std::unordered_map<std::string, Creator> creators_;
};
