#include "../model/basic_geometry.h"
#include "../model/document.h"
#include "../model/shape.h"
#include "../model/shape_id.h"
#include "../model/shape_factory.h"
#include "../view/view.h"
#include "../controller/controller.h"
#include "../io/serializer.h"

#include <iostream>
#include <string>
#include <memory>

using namespace document;
using namespace shape;
using namespace shape_id;
using namespace view;
using namespace controller;
using namespace serializer;

// Handler: File → New
void new_document(const EditorController &ctl) {
    ctl.new_document();
}

// Handler: File → Import...
void import_file(const EditorController &ctl, const std::string &path) {
    if (!ctl.import_document(path)) {
        std::cerr << "Import failed for: " << path << "\n";
    }
}

// Handler: File → Export...
void export_file(const EditorController &ctl, const std::string &path) {
    if (!ctl.export_document(path)) {
        std::cerr << "Export failed for: " << path << "\n";
    }
}

// Handler: Insert → Shape
ShapeId create_shape(const EditorController &ctl, const std::string &type) {
    try {
        return ctl.create_primitive(type);
    } catch (const std::exception &e) {
        std::cerr << "Create shape failed: " << e.what() << "\n";
        return ShapeId{};
    }
}

// Handler: Edit → Delete
void delete_shape(const EditorController &ctl, const ShapeId id) {
    if (!ctl.delete_primitive(id)) {
        std::cerr << "Delete shape failed: id=" << id.value << "\n";
    }
}

// ----------------------------------- main ------------------------------------
/*
 * The main() function wires MVC and simulates a short user session by calling
 * the GUI handlers. Replace handlers with real UI callbacks in an actual app.
 */
int main() {
    // 1) Build Model, View, Controller objects.
    const auto doc = std::make_shared<Document>();
    const auto view = std::make_shared<ConsoleView>();
    const auto factory = std::make_shared<ShapeFactory>();
    auto io = std::make_unique<DummySerializer>();

    // 2) Register available shape creators in the factory (extensible).
    factory->register_type("Line", [] {
        return std::make_shared<Line>(Point{0, 0}, Point{100, 100});
    });
    factory->register_type("Rectangle", [] {
        return std::make_shared<Rectangle>(Point{10, 10}, 50, 30);
    });
    factory->register_type("Circle", [] {
        return std::make_shared<Circle>(Point{40, 40}, 20.0);
    });
    factory->register_type("Huge Circle", [] {
        return std::make_shared<Circle>(Point{0, 0}, 20000.0);
    });


    // 3) Create the Controller.
    const EditorController controller{doc, view, std::move(io), factory};

    // ---- Simulated GUI Session (Handlers are the "GUI callbacks") ----

    // File → New
    new_document(controller);

    // Insert → Rectangle
    create_shape(controller, "Rectangle");

    // Insert → Circle
    create_shape(controller, "Circle");
    create_shape(controller, "Huge Circle");

    // File → Export...
    export_file(controller, "scene.mock");

    // File → New → (clear) then File → Import...
    new_document(controller);
    import_file(controller, "scene.mock");

    // Insert → Line
    const auto lId = create_shape(controller, "Line");

    // Edit → Delete (remove the circle we created earlier, if any)
    if (lId.value != 0) delete_shape(controller, lId);

    // Final export to show resulting state
    export_file(controller, "scene_final.mock");
    return 0;
}
