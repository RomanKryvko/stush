#pragma once
#include <cstdint>

struct cursor_pos {
    cursor_pos() = default;
    cursor_pos(int row, int col) : row(row), col(col) {}
    int row {};
    int col {};
};

using key_code_t = int32_t;

