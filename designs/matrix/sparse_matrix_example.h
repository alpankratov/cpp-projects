#pragma once
#include <cstddef>
#include <iterator>
#include <algorithm>
#include <unordered_map>

class SparseMatrix {
public:
    explicit SparseMatrix() = default;

    explicit SparseMatrix(const int default_value) : def_(default_value) {
    }

    [[nodiscard]] std::size_t size() const noexcept { return data_.size(); }

    auto count_values() {
        return std::count_if(this->begin(), this->end(), [this](const auto &e) { return e.value != def_; });
    }

    class RowProxy;

    // to enable 2D array syntax matrix[row][col] for accessing elements in the sparse matrix - getting row
    RowProxy operator[](std::size_t row);
    RowProxy operator[](std::size_t row) const;

    struct Entry {
        std::size_t x;
        std::size_t y;
        int value;
    };

    class iterator {
        // iterating over cells (inner iterator) and rows (outer iterator)
        using OuterIt = std::unordered_map<std::size_t, std::unordered_map<std::size_t, int> >::const_iterator;
        using InnerIt = std::unordered_map<std::size_t, int>::const_iterator;

        const SparseMatrix *parent_;
        OuterIt outer_;
        InnerIt inner_;

        void advance_to_next_valid() {
            while (outer_ != parent_->data_.end() && inner_ == outer_->second.end()) {
                ++outer_;
                if (outer_ != parent_->data_.end()) inner_ = outer_->second.begin();
            }
        }

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = Entry;
        using difference_type = std::ptrdiff_t;
        using pointer = const Entry *;
        using reference = const Entry &;

        iterator(const SparseMatrix *p, const OuterIt o, const InnerIt i)
            : parent_(p), outer_(o), inner_(i) { advance_to_next_valid(); }

        iterator &operator++() {
            ++inner_;
            if (inner_ == outer_->second.end()) {
                ++outer_;
                if (outer_ != parent_->data_.end())
                    inner_ = outer_->second.begin();
            }
            advance_to_next_valid();
            return *this;
        }

        Entry operator*() const {
            return Entry{outer_->first, inner_->first, inner_->second};
        }

        bool operator==(const iterator &other) const {
            return outer_ == other.outer_ && (outer_ == parent_->data_.end() || inner_ == other.inner_);
        }

        bool operator!=(const iterator &other) const { return !(*this == other); }
    };

    iterator begin() const noexcept {
        return iterator(this, data_.begin(),
                        data_.empty()
                            ? std::unordered_map<std::size_t, int>::const_iterator()
                            : data_.begin()->second.begin());
    }

    iterator end() const noexcept { return iterator(this, data_.end(), {}); }

    class RowProxy {
        SparseMatrix &parent_;
        std::size_t row_;

    public:
        RowProxy(SparseMatrix &p, const std::size_t r) : parent_(p), row_(r) {
        }

        class CellProxy {
            SparseMatrix &parent_;
            std::size_t row_, col_;

        public:
            CellProxy(SparseMatrix &p, const std::size_t r, const std::size_t c)
                : parent_(p), row_(r), col_(c) {
            }

            /* read */
            operator int() const { return parent_.get(row_, col_); }

            /* write – returns *this* to allow chaining */
            CellProxy &operator=(const int &v) {
                parent_.set(row_, col_, v);
                return *this;
            }

            /* enable compound‑assignments if desired */
            CellProxy &operator+=(const int &v) {
                int cur = parent_.get(row_, col_);
                parent_.set(row_, col_, cur + v);
                return *this;
            }

            // … similarly for -=, *=, /= …
        };

        CellProxy operator[](const std::size_t col) { return CellProxy(parent_, row_, col); }

        CellProxy operator[](const std::size_t col) const {
            return CellProxy(const_cast<SparseMatrix &>(parent_), row_, col);
        }
    };

private:
    int def_{0}; // default value

    std::unordered_map<std::size_t, std::unordered_map<std::size_t, int> > data_; // row → (column → value)

    int get(const std::size_t r, const std::size_t c) const {
        auto it_row = data_.find(r);
        if (it_row != data_.end()) {
            auto it_col = it_row->second.find(c);
            if (it_col != it_row->second.end())
                return it_col->second;
        }
        return def_;
    }

    void set(const std::size_t r, const std::size_t c, const int &v) {
        if (v == def_) {
            // erase if present
            auto it_row = data_.find(r);
            if (it_row != data_.end()) {
                it_row->second.erase(c);
                if (it_row->second.empty())
                    data_.erase(it_row);
            }
        }
        else {
            data_[r][c] = v;
        }
    }
};

// Implementation of operator[] methods after RowProxy is fully defined
inline SparseMatrix::RowProxy SparseMatrix::operator[](const std::size_t row) {
    return RowProxy{*this, row};
}

inline SparseMatrix::RowProxy SparseMatrix::operator[](const std::size_t row) const {
    return RowProxy{const_cast<SparseMatrix &>(*this), row};
}


// ----- number of occupied cells -----
// std::cout << "\nOccupied cells: " << matrix.count_values() << "\n";

// ----- list all occupied cells -----
// std::cout << "List (x y value):\n";
// for (auto [x, y, value]: matrix) {
//     std::cout << x << ' ' << y << ' ' << value << '\n';
// }
