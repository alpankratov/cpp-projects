#pragma once
#include <cstddef>
#include <functional>
#include <unordered_map>


template<class T>
class SparseMatrix {
public:
    using Coordinates = std::pair<std::size_t, std::size_t>;
    using CellValue = T;

    explicit SparseMatrix() = default;

    explicit SparseMatrix(T default_value) : def_(default_value) {
    }

    [[nodiscard]] std::size_t size() const noexcept { return data_.size(); }

    // to enable 2D array syntax matrix[row][col] for accessing elements in the sparse matrix - getting row
    class RowProxy;
    RowProxy operator[](std::size_t row) { return RowProxy(*this, row); }
    // do also const implementation as usual
    RowProxy operator[](std::size_t row) const { return RowProxy(const_cast<SparseMatrix &>(*this), row); }

    struct Entry {
        std::size_t x, y;
        T value;
    };

    class iterator {
        // iterating over cells (inner iterator) and rows (outer iterator)
        using OuterIt = typename std::unordered_map<std::size_t, std::unordered_map<std::size_t, T> >::const_iterator;
        using InnerIt = typename std::unordered_map<std::size_t, T>::const_iterator;

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

        iterator(const SparseMatrix *p, OuterIt o, InnerIt i)
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
                            ? typename std::unordered_map<std::size_t, T>::const_iterator()
                            : data_.begin()->second.begin());
    }

    iterator end() const noexcept { return iterator(this, data_.end(), {}); }

private:
    T def_{-1}; // default value

    std::unordered_map<std::size_t, std::unordered_map<std::size_t, T> > data_; // row → (column → value)

    friend class RowProxy;

    T get(std::size_t r, std::size_t c) const {
        auto it_row = data_.find(r);
        if (it_row != data_.end()) {
            auto it_col = it_row->second.find(c);
            if (it_col != it_row->second.end())
                return it_col->second;
        }
        return def_;
    }

    void set(std::size_t r, std::size_t c, const T &v) {
        if (v == def_) {
            // erase if present
            auto itRow = data_.find(r);
            if (itRow != data_.end()) {
                itRow->second.erase(c);
                if (itRow->second.empty())
                    data_.erase(itRow);
            }
        }
        else {
            data_[r][c] = v;
        }
    }
};

template<class T>
class SparseMatrix<T>::RowProxy {
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
        operator T() const { return parent_.get(row_, col_); }

        /* write – returns *this* to allow chaining */
        CellProxy &operator=(const T &v) {
            parent_.set(row_, col_, v);
            return *this;
        }

        /* enable compound‑assignments if desired */
        CellProxy &operator+=(const T &v) {
            T cur = parent_.get(row_, col_);
            parent_.set(row_, col_, cur + v);
            return *this;
        }

        // … similarly for -=, *=, /= …
    };

    CellProxy operator[](std::size_t col) { return CellProxy(parent_, row_, col); }
    CellProxy operator[](std::size_t col) const { return CellProxy(const_cast<SparseMatrix &>(parent_), row_, col); }
};
