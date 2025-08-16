/*
* The Controller is the orchestrator between GUI events and the Model+View.
 * - It exposes high-level actions: new/import/export/create/delete
 * - It ensures the View is refreshed after each state-changing operation
 * - It does not *render* and does not store GUI state (SRP)
 * - It can switch serialization strategies at runtime
 */
#pragma once
#include "../model/document.h"
#include "../model/shape_id.h"
#include "../model/shape_factory.h"
#include "../view/view.h"
#include "../io/serializer.h"
#include <memory>

using namespace shape_id;
using namespace serializer;
using namespace view;
using namespace document;

namespace controller {
    class EditorController {
    public:
        EditorController(std::shared_ptr<Document> doc,
                         std::shared_ptr<IView> view,
                         std::unique_ptr<ISerializer> serializer,
                         std::shared_ptr<ShapeFactory> factory)
            : doc_(std::move(doc)), view_(std::move(view)),
              serializer_(std::move(serializer)), factory_(std::move(factory)) {
        }

        // --------------------------- Application-level actions --------------------

        void new_document() {
            doc_->clear();
            notify_view();
        }

        bool import_document(const std::string &file) {
            const bool ok = serializer_->import_from_file(*doc_, file, *factory_);
            notify_view();
            return ok;
        }

        bool export_document(const std::string &file) {
            return serializer_->export_to_file(*doc_, file);
        }

        ShapeId create_primitive(std::string type) {
            auto shape = factory_->create(type);
            const auto id = doc_->add_shape(std::move(shape));
            notify_view();
            return id;
        }

        bool delete_primitive(ShapeId id) {
            const bool ok = doc_->remove_shape(id);
            notify_view();
            return ok;
        }

        // Optional: inject a new serializer at runtime (e.g., user selects SVG).
        void set_serializer(std::unique_ptr<ISerializer> s) { serializer_ = std::move(s); }

    private:
        void notify_view() { if (view_) view_->refresh(*doc_); }

        std::shared_ptr<Document> doc_;
        std::shared_ptr<IView> view_;
        std::unique_ptr<ISerializer> serializer_;
        std::shared_ptr<ShapeFactory> factory_;
    };
}
