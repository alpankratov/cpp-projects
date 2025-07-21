#pragma once
#include "custom_allocators.h"

namespace custom_containers {
    template<typename T, typename Allocator = custom_allocators::CustomAllocator<T> >
    class CustomVector {
    private:
        std::size_t capacity_;
        Allocator allocator_;
        T *data_;
        std::size_t size_;

    public:
        CustomVector() : allocator_{10},
                         capacity_{10},
                         data_{allocator_.allocate(static_cast<uint>(capacity_))},
                         size_{0} {
            std::cout << "Custom Vector initialized with no parameter - default capacity of 10 is used\n";
        }

        // Construct vector with an initial capacity:
        explicit CustomVector(const std::size_t capacity)
            : allocator_{static_cast<uint>(capacity)},
              data_{allocator_.allocate(static_cast<uint>(capacity))},
              size_{0},
              capacity_{capacity} {
        }


        ~CustomVector() {
            for (std::size_t i = 0; i < size_; ++i)
                data_[i].~T();

            if (data_)
                allocator_.deallocate(data_, capacity_);
        }

        void push_back(const T &value) {
            if (size_ > capacity_)
                // custom vector is constructed with fixed capacity that cannot be changed;
                throw std::overflow_error("Vector capacity is exhausted");
            new(data_ + size_) T(value);
            ++size_;
        }

        void pop_back() {
            if (size_ == 0)
                throw std::underflow_error("Vector is empty");

            data_[--size_].~T();
        }

        T &operator[](std::size_t index) {
            if (index >= size_)
                throw std::out_of_range("Index out of range");
            return data_[index];
        }

        const T &operator[](std::size_t index) const {
            if (index >= size_)
                throw std::out_of_range("Index out of range");
            return data_[index];
        }

        std::size_t size() const { return size_; }
        std::size_t capacity() const { return capacity_; }
    };
}
