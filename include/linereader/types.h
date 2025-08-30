#pragma once
#include <cstdint>

struct cursor_pos {
    cursor_pos() = default;
    cursor_pos(int row, int col) : row(row), col(col) {}
    bool operator==(cursor_pos other) const {
        return this->col == other.col && this->row == other.row;
    }

    int row {};
    int col {};
};

using key_code_t = int32_t;

