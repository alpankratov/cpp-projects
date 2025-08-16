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
            std::cout << "[View] Document has " << doc.list().size() << " shapes:\n";
            int idx = 0;
            for (auto &s: doc.list()) {
                const auto b = s->bounds();
                std::cout << "  #" << idx++ << " " << s->type()
                        << " bounds=(" << b.x << "," << b.y << "," << b.w << "," << b.h << ")\n";
            }
        }
    };
}
