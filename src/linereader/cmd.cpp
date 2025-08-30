#include "linereader/terminal.h"
#include <linereader/cmd.h>
#include <byteutils.h>
#include <parser.h>
#include <cassert>
#include <iostream>
#include <sched.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>
#include <wait.h>
#include <termios.h>

cursor_pos current_cursor {};

// starting column position
int line_start = 1;

Terminal term {};
std::string line_state {};
std::string _prompt {};

void set_cursor_position(cursor_pos position) {
    current_cursor = position;
    term.set_cursor_position(position);
}

void redraw_line(const std::string& prompt, const std::string& line) {
    term.erase_line();
    term.set_cursor_position({current_cursor.row, 0});
    term.write_text(prompt + line);
    term.set_cursor_position(current_cursor);
}

inline void move_cursor_right() {
    term.move_cursor_right();
    current_cursor.col++;
}

inline void move_cursor_left() {
    term.move_cursor_left();
    current_cursor.col--;
}

int full_line_length() {
    return line_state.size() + line_start;
}

void go_to_line_start() {
    set_cursor_position({current_cursor.row, line_start});
}

void go_to_line_end() {
    set_cursor_position({current_cursor.row, static_cast<int>(line_state.size()) + line_start});
}

void erase_to_beginning() {
    line_state.erase(0, current_cursor.col - line_start);
    term.erase_to_line_start();
    set_cursor_position({current_cursor.row, line_start});
    redraw_line(_prompt, line_state);
}

void erase_to_end() {
    if (full_line_length() >= current_cursor.col) {
        line_state.erase(current_cursor.col - line_start);
        term.erase_to_line_end();
    }
}

void erase_forward() {
    if (!line_state.empty() &&
        current_cursor.col > line_start &&
        current_cursor.col < full_line_length())
    {
        line_state.erase(current_cursor.col - line_start, 1);
        redraw_line(_prompt, line_state);
    }
}

void erase_backwards() {
    if (!line_state.empty() && current_cursor.col > line_start) {
        if (current_cursor.col < full_line_length()) {
            line_state.erase(current_cursor.col - line_start, 1);
        } else {
            line_state.pop_back();
        }
        move_cursor_left();
        redraw_line(_prompt, line_state);
    }
}

void cursor_up() {
    // Do nothing.
    // TODO: implement history lookup here
}

void cursor_down() {
    // Do nothing.
    // TODO: implement history lookup here
}

void cursor_right() {
    if (current_cursor.col < full_line_length()) {
        move_cursor_right();
    }
}

void cursor_left() {
    if (current_cursor.col > line_start) {
        move_cursor_left();
    }
}

std::unordered_map<key_code_t, command_t> command_map {
    {packn<key_code_t>(0, 0, 0, CTRL_A), go_to_line_start},
    {packn<key_code_t>(0, 0, 0, CTRL_E), go_to_line_end},
    {packn<key_code_t>(0, 0, 0, CTRL_U), erase_to_beginning},
    {packn<key_code_t>(0, 0, 0, CTRL_K), erase_to_end},
    {packn<key_code_t>(0, 0, 0, CTRL_H), erase_backwards},
    {packn<key_code_t>(0, 0, 0, BACKSPACE), erase_backwards},
    {packn<key_code_t>(0x7e, 0x33, 0x5b, 0x1b), erase_forward},
    {packn<key_code_t>(0, 0x41, 0x5b, 0x1b), cursor_up},
    {packn<key_code_t>(0, 0x42, 0x5b, 0x1b), cursor_down},
    {packn<key_code_t>(0, 0x43, 0x5b, 0x1b), cursor_right},
    {packn<key_code_t>(0, 0x44, 0x5b, 0x1b), cursor_left},
};

void init_readline(std::string_view prompt) {
    line_state.clear();
    _prompt = prompt;
    term.enable_raw_mode();
    line_start = _prompt.size() + 1;
    std::cout << "\r\n" << _prompt << std::flush;
    current_cursor = term.query_cursor_position();
}

void handle_control(key_code_t key) {
    try {
        command_map.at(key)();
    } catch (const std::out_of_range& e) {
        // Not implemented. Do nothing?
    }
}

void handle_normal(key_code_t key) {
    if (current_cursor.col < full_line_length()) {
        line_state.insert(current_cursor.col - line_start, 1, key);
    } else {
        line_state += key;
    }
    redraw_line(_prompt, line_state);
    move_cursor_right();
}

std::string sh_read_line(std::string_view prompt, char terminator) {
    init_readline(prompt);
    key_code_t key {term.read_key()};
    while (key != terminator) {
        if (iscntrl(key & 0xFF)) { // check lowest byte
            handle_control(key);
        } else {
            handle_normal(key);
        }
        term.commit();
        key = term.read_key();
    }
    term.disable_raw_mode();
    return line_state;
}
