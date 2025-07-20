#pragma once
#include <cstdint>
#include <iostream>

namespace custom_allocators {
    template<typename T>
    class CustomAllocator {
    private:
        // Our memory pool - pre-allocated chunk
        char *pool_{};
        size_t pool_size_{};
        char *next_free_{}; // Points to next available memory
        uint n_elements_{};

        uint max_n_elements_ = 10;

        // Initialize pool on first use
        void init_pool() {
            if (!pool_) {
                if (n_elements_ == 0) {
                    n_elements_ = max_n_elements_;
                    std::cout << "Allocator was initialized with no elements, "
                            "therefore " << max_n_elements_ << " are being set\n";
                }
                if (n_elements_ > max_n_elements_) {
                    throw std::runtime_error("Too many elements");
                }
                std::cout << "Initializing memory pool for "
                        << n_elements_ << " elements " << sizeof(T) << " bytes each\n";
                const size_t POOL_SIZE = n_elements_ * sizeof(T);
                pool_ = static_cast<char *>(std::malloc(POOL_SIZE));
                next_free_ = pool_;
                pool_size_ = POOL_SIZE;
                std::cout << "Created memory pool of " << POOL_SIZE << " bytes\n";
            }
        }

    public:
        using value_type = T;

        CustomAllocator() = default;

        explicit CustomAllocator(const uint n_elements) : pool_(nullptr), pool_size_(0), next_free_(nullptr),
                                                          n_elements_(n_elements) {
            init_pool();
        };

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
