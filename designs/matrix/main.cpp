#include <iostream>
#include <ostream>

#include "sparse_matrix.h"
using namespace sparse_matrix;

int main() {
    SparseMatrix matrix{0}; // default value = 0

    // filling in main diagonal with 0..9
    for (std::size_t i = 0; i <= 9; ++i)
        matrix[i][i] = static_cast<int>(i);

    // filling secondary diagonal with 9..0
    for (std::size_t i = 0; i <= 9; ++i)
        matrix[i][9 - i] = static_cast<int>(9 - i);

    // printing fragment [1,1] .. [8,8]
    for (std::size_t r = 1; r <= 8; ++r) {
        for (std::size_t c = 1; c <= 8; ++c) {
            std::cout << matrix[r][c];
            if (c != 8) std::cout << ' ';
        }
        std::cout << '\n';
    }

    // printing number of occupied cells
    std::cout << "\nOccupied cells: " << matrix.count_values() << "\n";
    //
    // listing all occupied cells
    std::cout << "List (row column value):\n";
    for (auto [x, y, value]: matrix) {
        std::cout << x << ' ' << y << ' ' << value << '\n';
    }

    return 0;
}
