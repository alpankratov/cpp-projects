#pragma once
#include <unordered_map>
#include <vector>

namespace sparse_matrix {
    class SparseMatrix {
    public:
        SparseMatrix() = default;
        explicit SparseMatrix(const int default_value) : default_(default_value) {
        }

        class Row {
            SparseMatrix &parent_;
            std::size_t row_;
            public:
            Row(SparseMatrix &p, const std::size_t r) : parent_(p), row_(r) {
            }
            class Cell {
                SparseMatrix &parent_;
                std::size_t row_, col_;
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
            Cell operator[](const std::size_t col) const { return Cell{const_cast<SparseMatrix &>(parent_), row_, col}; }
        };
        Row operator[](const std::size_t row) { return Row{*this, row}; }
        Row operator[](const std::size_t row) const { return Row{const_cast<SparseMatrix &>(*this), row}; }

    private:
        int default_ = 0;
        // so matrix is a hashed map of row of a hashed map of columns containing values
        // i.e. {4: {2: 1}} -> value 1 is located in 4th row and 2nd column.
        std::unordered_map<std::size_t, std::unordered_map<std::size_t, int> > data_;
        int get(const std::size_t row, const std::size_t col) const {
            auto it_row = data_.find(row);  // check if a row is non-empty (so data_ has vales)
            if (it_row != data_.end()) {
                auto it_col = it_row->second.find(col);  // check if a column is non-empty (so data_ has vales)
                if (it_col != it_row->second.end())
                    return it_col->second;  // extracting value
            }
            return default_;  // if nothing is identified -> then return default value (as it is a sparse matrix)
        }

        void set(const std::size_t row, const std::size_t col, const int &v) {
            if (v != default_) {
                data_[row][col] = v;  // if not default -> then assign value into the cell
            }
            else {
                // if default value, we need to erase the value from the matrix to align with the logic of
                // a sparse matrix filled in with default values
                auto it_row = data_.find(row);
                if (it_row != data_.end()) {
                    it_row->second.erase(col);
                    if (it_row->second.empty())
                        data_.erase(it_row);
                }
            }
        }

    };
}
