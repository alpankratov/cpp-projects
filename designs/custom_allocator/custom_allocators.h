#pragma once
#include <cstdint>
#include <iostream>
#include <sstream>

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
                    std::ostringstream oss;
                    oss << "Too many elements requested: "
                        << n_elements_
                        << " (max allowed "
                        << max_n_elements_
                        << ")";
                    throw std::runtime_error(oss.str());
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

        // we allow constructing this allocator only for a specific number of elements in a container
        CustomAllocator() = delete;

        explicit CustomAllocator(const uint n_elements) : n_elements_(n_elements) {
        };

        auto get_n_elements() const {
            return n_elements_;
        }

        // to “rebind” allocator to other types (e.g., when std::map key-value pair is assigned)
        template<typename U>
        constexpr explicit CustomAllocator(const CustomAllocator<U> &other) noexcept {
            n_elements_ = other.get_n_elements();
        }


        T *allocate(std::size_t n) {
            init_pool();
            size_t bytes_needed = n * sizeof(T);
            std::cout << "Trying to allocate " << n << " x " << sizeof(T) << " bytes\n";

            // Check if we have enough space in our pool
            if (next_free_ + bytes_needed > pool_ + pool_size_) {
                std::cout << "Pool exhausted! Falling back to malloc\n";
                return static_cast<T *>(std::malloc(bytes_needed));
            }

            // Give memory from our pool
            T *result = reinterpret_cast<T *>(next_free_);
            next_free_ += bytes_needed;

            std::cout << "Allocated " << bytes_needed << " bytes from pool\n";
            std::cout << "Pool usage: " << (next_free_ - pool_) << "/" << pool_size_ << " bytes\n";
            return result;
        }

        void deallocate(T *p, std::size_t n) noexcept {
            std::size_t bytes = n * sizeof(T);
            auto cp = reinterpret_cast<char *>(p);

            // If pointer lies within our pool range, simply ignore (bump allocator)
            if (pool_ && cp >= pool_ && cp < pool_ + pool_size_) {
                std::cout << "Deallocated " << bytes << " bytes back to pool\n";
                next_free_ -= bytes;
                std::cout << "Pool usage: " << (next_free_ - pool_) << "/" << pool_size_ << " bytes\n";
            } else {
                // Otherwise it was malloc'd as a fallback
                std::cout << "Deallocated " << bytes << " bytes from fallback malloc\n";
                std::free(p);
            }
        }

        // Reset entire pool (custom function)
        void reset_pool() {
            if (pool_) {
                next_free_ = pool_;
                std::cout << "Pool reset - all memory available again\n";
            }
        }

        void cleanup() {
            if (pool_) {
                std::free(pool_);
                pool_ = nullptr;
                std::cout << "Pool destroyed\n";
            }
        }
    };
}
