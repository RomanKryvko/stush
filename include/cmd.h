#pragma once

#include <string_view>
#include <string>
#include <termios.h>

struct cursor_pos {
    cursor_pos() = default;
    cursor_pos(int row, int col) : row(row), col(col) {}
    int row {};
    int col {};
};

const char CTRL_A = CTRL('a');
const char CTRL_E = CTRL('e');
const char CTRL_U = CTRL('u');
const char CTRL_K = CTRL('k');
const char CTRL_H = CTRL('h');
const char BACKSPACE = 0x7f;
const char ENTER = 0xd;
const char ESC = 0x1b;

void disable_raw_mode();

void enable_raw_mode();

cursor_pos query_cursor_pos();

std::string set_cursor_position(cursor_pos position);

std::string redraw_line(std::string_view prompt, std::string_view line);

std::string move_cursor_right();

std::string move_cursor_left();

std::string sh_read_line(std::string_view prompt);
