#pragma once

#include "linereader/utfstring.h"
#include "stringsep.h"
#include "types.h"
#include <cstddef>
#include <string>

class LineBuffer {
    std::string _word_separators {sep::WORD_SEPARATORS};
    utf8string buffer {};
    utf8string yank_buffer {};
    int _line_start {1};
    cursor_pos cursor {};

    inline int full_line_length() const;

    inline size_t idx_to_cursor() const;
    inline size_t idx_to_cursor(int col) const;
    inline size_t cursor_to_idx() const;

    inline void cut(size_t pos, size_t n);

public:
    void line_start(int col);
    int line_start() const;

    void word_separators(const std::string& separators);
    const std::string& word_separators() const;

    const std::string& get_text() const;
    void set_text(const std::string& text);
    bool insert(key_code_t key);

    cursor_pos cursor_position() const;
    void cursor_position(cursor_pos position);

    bool move_cursor_right();
    bool move_cursor_left();

    void jump_word_right();
    void jump_word_left();

    void go_to_line_start();
    void go_to_line_end();

    bool erase_to_beginning();
    bool erase_to_end();
    bool erase_forward();
    bool erase_backwards();

    bool erase_word_backwards();
    bool erase_word_forward();

    bool paste();
};
