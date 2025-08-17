/*
* View contract: Controller calls refresh() when Model changes.
 * Real apps might pass a const Document& and render it with a graphics API.
 */
#pragma once
#include <iostream>
#include "../model/document.h"

using namespace document;

namespace view {
    class IView {
    public:
        virtual ~IView() = default;

        virtual void refresh(const Document &doc) = 0;
    };

    class ConsoleView final : public IView {
    public:
        void refresh(const Document &doc) override {
            const auto doc_map = doc.parse_into_map();
            std::cout << "[View] Refreshing view... Document now has " << doc_map.size() << " shapes:\n";
            for (const auto &[shape_id, shape]: doc.parse_into_map()) {
                std::cout << "Shape id " << shape_id << ": ";
                shape->explain();
            }
        }
    };
}
