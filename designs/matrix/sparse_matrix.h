#pragma once
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace sparse_matrix {
    class SparseMatrix {
    public:
        SparseMatrix() = default;

        explicit SparseMatrix(const int default_value) : default_(default_value) {
        }

        // A more friendly presentation of unordered map data_ that is used for accessing to values when iterating
        // over matrix
        struct Entry {
            std::size_t x;
            std::size_t y;
            int         value;
        };

        class iterator {
            // iterating over cells (inner iterator) and rows (outer iterator)
            using OuterIt = std::unordered_map<std::size_t, std::unordered_map<std::size_t, int> >::const_iterator;
            using InnerIt = std::unordered_map<std::size_t, int>::const_iterator;

            const SparseMatrix *parent_ = nullptr;
            OuterIt             outer_;
            InnerIt             inner_;

            void advance_to_next_valid() {
                // Skip empty rows to ensure we point to actual data
                while (outer_ != parent_->data_.end() && (outer_->second.empty() || inner_ == outer_->second.end())) {
                    ++outer_;
                    if (outer_ != parent_->data_.end()) inner_ = outer_->second.begin();
                }
            }

        public:
            // minimum requirements for an iterator
            // 1. Types that describe what the iterator points to and how it moves
            // value_type, reference, pointer, difference_type, iterator_category
            using value_type = Entry;
            using reference = const Entry;
            using pointer = const Entry *; // not used only for future reference
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::forward_iterator_tag; // need to move only forward through the matrix
            // Note regarding the reference: sparse matrix doesn't actually store `Entry` objects anywhere in memory.
            // The `Entry` is just a convenient way to package the row index, column index, and value together
            // when iterating. Since there's no actual object to reference, Entry is returned by value.


            // 2. Constructors (copy‑constructible / assignable)
            iterator() = default;

            iterator(const SparseMatrix *p, const OuterIt o, const InnerIt i)
                : parent_(p), outer_(o), inner_(i) {
            }

            // 3. Dereference: operator*() (and optionally operator->()) -> Gives access to the element.
            reference operator*() const {
                return Entry{outer_->first, inner_->first, inner_->second};
            }

            // 4. Increment: operator++() -> Moves the iterator to the next element.
            // PS: No decrement operator implemented in this assignment
            iterator &operator++() {
                ++inner_;
                if (inner_ == outer_->second.end()) {
                    ++outer_;
                    if (outer_ != parent_->data_.end()) inner_ = outer_->second.begin();
                }
                advance_to_next_valid();
                return *this;
            }

            // 5. Equality: operator==(const iterator& other) -> Checks if the iterators point to the same element.
            bool operator==(const iterator &other) const {
                return outer_ == other.outer_ && (inner_ == other.inner_ || outer_ == parent_->data_.end());
            }

            bool operator!=(const iterator &other) const { return !(*this == other); }
        };

        iterator begin() const noexcept {
            if (data_.empty()) return end();
            return iterator{this, data_.begin(), data_.begin()->second.begin()};
        }

        iterator end() const noexcept { return iterator{this, data_.end(), {}}; }

        auto count_values() {
            return std::count_if(this->begin(), this->end(), [this](const auto &e) { return e.value != default_; });
        }


        class Row {
            SparseMatrix &parent_;
            std::size_t   row_;

        public:
            Row(SparseMatrix &p, const std::size_t r) : parent_(p), row_(r) {
            }

            class Cell {
                SparseMatrix &parent_;
                std::size_t   row_, col_;

            public:
                Cell(SparseMatrix &p, const std::size_t r, const std::size_t c)
                    : parent_(p), row_(r), col_(c) {
                }

                // to read a cell from the matrix
                // convert to int to enable **transparent reading** from the sparse matrix
                // (i.e. matrix[r][c] returning int and not Cell when client expects int)
                operator int() const { return parent_.get(row_, col_); }
                // to write a cell to the matrix
                Cell &operator=(const int &v) {
                    parent_.set(row_, col_, v);
                    return *this;
                }
            };

            Cell operator[](const std::size_t col) { return Cell{parent_, row_, col}; }

            Cell operator[](const std::size_t col) const {
                return Cell{const_cast<SparseMatrix &>(parent_), row_, col};
            }
        };

        Row operator[](const std::size_t row) { return Row{*this, row}; }
        Row operator[](const std::size_t row) const { return Row{const_cast<SparseMatrix &>(*this), row}; }

    private:
        int default_ = 0;
        // so matrix is a hashed map of row of a hashed map of columns containing values
        // i.e. {4: {2: 1}} -> value 1 is located in 4th row and 2nd column.
        std::unordered_map<std::size_t, std::unordered_map<std::size_t, int> > data_;

        int get(const std::size_t row, const std::size_t col) const {
            auto it_row = data_.find(row); // check if a row is non-empty (so data_ has vales)
            if (it_row != data_.end()) {
                auto it_col = it_row->second.find(col); // check if a column is non-empty (so data_ has vales)
                if (it_col != it_row->second.end()) return it_col->second; // extracting value
            }
            return default_; // if nothing is identified -> then return default value (as it is a sparse matrix)
        }

        void set(const std::size_t row, const std::size_t col, const int &v) {
            if (v != default_) {
                data_[row][col] = v; // if not default -> then assign value into the cell
            }
            else {
                // if default value, we need to erase the value from the matrix to align with the logic of
                // a sparse matrix filled in with default values
                auto it_row = data_.find(row);
                if (it_row != data_.end()) {
                    it_row->second.erase(col);
                    if (it_row->second.empty()) data_.erase(it_row);
                }
            }
        }
    };
}
