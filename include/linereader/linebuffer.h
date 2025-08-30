#pragma once

#include "linereader.h"
#include <string>

class LineBuffer {
    std::string buffer {};
    int _line_start {1};
    cursor_pos cursor {};

    int full_line_length();

public:
    void line_start(int col);
    int line_start() const;

    const std::string& get_text() const;
    void set_text(const std::string& text);
    void insert(key_code_t key);

    cursor_pos cursor_position() const;
    void cursor_position(cursor_pos position);

    bool move_cursor_right();
    bool move_cursor_left();

    void go_to_line_start();
    void go_to_line_end();

    bool erase_to_beginning();
    bool erase_to_end();
    bool erase_forward();
    bool erase_backwards();
};
