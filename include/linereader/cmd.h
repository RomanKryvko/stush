#pragma once

#include <string_view>
#include <termios.h>

using command_t = void(*)(void);

const char CTRL_A = CTRL('a');
const char CTRL_E = CTRL('e');
const char CTRL_U = CTRL('u');
const char CTRL_K = CTRL('k');
const char CTRL_H = CTRL('h');
const char BACKSPACE = 0x7f;
const char ENTER = 0xd;
const char ESC = 0x1b;

void redraw_line(const std::string& prompt, const std::string& line);

void move_cursor_right();

void move_cursor_left();

std::string sh_read_line(std::string_view prompt, char terminator = ENTER);
