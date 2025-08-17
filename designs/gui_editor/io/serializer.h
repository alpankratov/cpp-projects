/*
* Polymorphic strategy for import/export of files. Only the dummy case is implemented for demo purposes
 */
#pragma once
#include "../model/document.h"
#include "../model/shape.h"
#include "../model/shape_factory.h"
#include "../model/basic_geometry.h"
#include <iostream>
#include <string>
#include <memory>

using namespace document;
using namespace shape;

namespace serializer {
    class ISerializer {
    public:
        virtual ~ISerializer() = default;

        virtual bool export_to_file(const Document &doc, const std::string &path) = 0;

        virtual bool import_from_file(Document &doc, const std::string &path, const ShapeFactory &factory) = 0;
    };

    class DummySerializer final : public ISerializer {
    public:
        bool export_to_file(const Document &doc, const std::string &path) override {
            // Mock: Just report to stdout to indicate the call succeeded.
            std::cout << "[DummySerializer] Exported document to " << path
                    << " (shapes: " << doc.parse_into_map().size() << ")\n";
            return true;
        }

        bool import_from_file(Document &doc, const std::string &path, const ShapeFactory &) override {
            // Mock: Pretend we read one rectangle from disk.
            (void) path;
            doc.clear();
            const auto rect = std::make_shared<Rectangle>(Point{10, 10}, 50, 40);
            doc.add_shape(rect);
            std::cout << "[DummySerializer] Imported document from " << path << "\n";
            return true;
        }
    };
}
