#pragma once

namespace custom_containers {
    template<typename T, typename Allocator = std::allocator<T> >
    class CustomVector {
    private:
        std::size_t capacity_;
        Allocator allocator_;
        T *data_;
        std::size_t size_;

    public:
        explicit CustomVector(
            std::size_t capacity = 10,
            const Allocator &alloc = Allocator()
        )
            : allocator_{alloc},
              capacity_{capacity},
              data_{allocator_.allocate(capacity_)},
              size_{0} {
            std::cout << "CustomVector initialized (capacity=" << capacity_ << ")\n";
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
