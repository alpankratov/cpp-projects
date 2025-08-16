#pragma once
#include "basic_geometry.h"
#include <memory>
#include <iostream>

namespace shape {
    class IShape {
    public:
        virtual ~IShape() = default;

        virtual std::unique_ptr<IShape> clone() const = 0;

        virtual std::string_view type() const noexcept = 0;

        virtual Rect bounds() const noexcept = 0;

        virtual void explain() const noexcept = 0;
    };

    class Line final : public IShape {
    public:
        Line(const Point a, const Point b) : a_{a}, b_{b} {
        }

        [[nodiscard]] std::unique_ptr<IShape> clone() const override { return std::make_unique<Line>(*this); }
        [[nodiscard]] std::string_view type() const noexcept override { return "Line"; }

        [[nodiscard]] Rect bounds() const noexcept override {
            const double minx = std::min(a_.x, b_.x), maxx = std::max(a_.x, b_.x);
            const double miny = std::min(a_.y, b_.y), maxy = std::max(a_.y, b_.y);
            return Rect{minx, miny, maxx - minx, maxy - miny};
        }

        [[nodiscard]] Point a() const noexcept { return a_; }
        [[nodiscard]] Point b() const noexcept { return b_; }

        void explain() const noexcept override {
            std::cout << "\nLine with coordinates (" << a_.x << "," << a_.y << ") and (" << b_.x << "," << b_.y <<
                    ")\n";
        }

    private:
        Point a_{}, b_{};
    };

    class Circle final : public IShape {
    public:
        Circle(const Point center, const double radius) : c_{center}, r_{radius} {
        }

        [[nodiscard]] std::unique_ptr<IShape> clone() const override { return std::make_unique<Circle>(*this); }
        [[nodiscard]] std::string_view type() const noexcept override { return "Circle"; }

        [[nodiscard]] Rect bounds() const noexcept override {
            return Rect{c_.x - r_, c_.y - r_, r_ * 2.0, r_ * 2.0};
        }

        void explain() const noexcept override {
            std::cout << "\nCircle with center at (" << c_.x << "," << c_.y << ") and radius of" << r_ << "\n";
        }

    private:
        Point c_{};
        double r_{};
    };

    // export is only for DummySerializer for mocking purpose
    class Rectangle final : public IShape {
    public:
        Rectangle(Point top_left, double w, double h) : top_left_{top_left}, w_{w}, h_{h} {
        }

        [[nodiscard]] std::unique_ptr<IShape> clone() const override { return std::make_unique<Rectangle>(*this); }
        [[nodiscard]] std::string_view type() const noexcept override { return "Rectangle"; }
        [[nodiscard]] Rect bounds() const noexcept override { return r_; }

        void explain() const noexcept override {
            std::cout << "\nRectangle starting from - top-left coordinate - (" << top_left_.x << "," << top_left_.x <<
                    ") width of " << w_ << " and width of " << h_ << "\n";
        }

    private:
        Point top_left_{};
        double w_{}, h_{};
        Rect r_{top_left_.x, top_left_.y, w_, h_};
    };
}
