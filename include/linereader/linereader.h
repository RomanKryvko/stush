#pragma once

#include "byteutils.h"
#include "linereader/linebuffer.h"
#include "linereader/terminal.h"
#include "linereader/types.h"
#include <functional>
#include <string_view>
#include <termios.h>
#include <unordered_map>

const char CTRL_A = CTRL('a');
const char CTRL_E = CTRL('e');
const char CTRL_U = CTRL('u');
const char CTRL_K = CTRL('k');
const char CTRL_H = CTRL('h');
const char CTRL_L = CTRL('l');
const char CTRL_W = CTRL('w');
const char CTRL_D = CTRL('d');
const char CTRL_Y = CTRL('y');
const char BACKSPACE = 0x7f;
const char ENTER = 0xd;
const char ESC = 0x1b;

class LineReader {
    using funcmap = std::unordered_map<key_code_t, std::function<void()>>;
    Terminal term {};
    LineBuffer linebuffer {};
    std::string _prompt {};
    funcmap command_map {
        {packn<key_code_t>(CTRL_A), std::bind(&LineReader::go_to_line_start, this)},
        {packn<key_code_t>(CTRL_E), std::bind(&LineReader::go_to_line_end, this)},
        {packn<key_code_t>(CTRL_U), std::bind(&LineReader::erase_to_beginning, this)},
        {packn<key_code_t>(CTRL_K), std::bind(&LineReader::erase_to_end, this)},
        {packn<key_code_t>(CTRL_H), std::bind(&LineReader::erase_word_backwards, this)},
        {packn<key_code_t>(BACKSPACE), std::bind(&LineReader::erase_backwards, this)},
        {packn<key_code_t>(CTRL_L), std::bind(&LineReader::clear, this)},
        {packn<key_code_t>(CTRL_W), std::bind(&LineReader::erase_word_backwards, this)},
        {packn<key_code_t>(CTRL_D), std::bind(&LineReader::erase_word_forward, this)},
        {packn<key_code_t>(CTRL_Y), std::bind(&LineReader::paste, this)},
        {packn<key_code_t>(0x7e, 0x33, 0x5b, ESC), std::bind(&LineReader::erase_forward, this)},
        {packn<key_code_t>(0x41, 0x5b, ESC), std::bind(&LineReader::cursor_up, this)},
        {packn<key_code_t>(0x42, 0x5b, ESC), std::bind(&LineReader::cursor_down, this)},
        {packn<key_code_t>(0x43, 0x5b, ESC), std::bind(&LineReader::move_cursor_right, this)},
        {packn<key_code_t>(0x44, 0x5b, ESC), std::bind(&LineReader::move_cursor_left, this)},
        {packn<key_code_t>('D', '5', ';', '1', 0x5b, ESC), std::bind(&LineReader::jump_word_left, this)},
        {packn<key_code_t>('C', '5', ';', '1', 0x5b, ESC), std::bind(&LineReader::jump_word_right, this)},
        {packn<key_code_t>('~', '5', ';', '3', 0x5b, ESC), std::bind(&LineReader::erase_word_forward, this)},
    };

    void set_cursor_position(cursor_pos position);

    void move_cursor_right();
    void move_cursor_left();
    void cursor_up();
    void cursor_down();
    void clear();

    void jump_word_left();
    void jump_word_right();

    void redraw_line(const std::string& prompt, const std::string& line);

    void erase_to_beginning();
    void erase_to_end();
    void erase_forward();
    void erase_backwards();

    void erase_word_forward();
    void erase_word_backwards();

    void go_to_line_start();
    void go_to_line_end();
    void paste();

    void init_readline(std::string_view prompt);

    void handle_control(key_code_t key);
    void handle_normal(key_code_t key);

public:
    std::string sh_read_line(std::string_view prompt, char terminator = ENTER);
};
