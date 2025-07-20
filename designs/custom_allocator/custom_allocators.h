#pragma once
#include <iostream>

namespace custom_allocators {
    template<typename T>
    class CustomAllocator {
    public:
        using value_type = T;

        CustomAllocator() = default;

        template<typename U>
        constexpr CustomAllocator(const CustomAllocator<U> &) noexcept {
        }

        T *allocate(std::size_t n) {
            std::cout << "[Allocate] " << n << " x " << sizeof(T) << " bytes\n";
            return static_cast<T *>(::operator new(n * sizeof(T)));
        }

        void deallocate(T *p, std::size_t n) noexcept {
            std::cout << "[Deallocate] " << n << " x " << sizeof(T) << " bytes\n";
            ::operator delete(p);
        }

        template<typename U, typename... Args>
        void construct(U *p, Args &&... args) {
            new(p) U(std::forward<Args>(args)...);
        }

        template<typename U>
        void destroy(U *p) {
            p->~U();
        }
    };
}
