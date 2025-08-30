#include "linereader/linebuffer.h"
#include "linereader/terminal.h"
#include <linereader/linereader.h>
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

Terminal term {};
LineBuffer linebuffer {};

std::string _prompt {};

void set_cursor_position(cursor_pos position) {
    linebuffer.cursor_position(position);
    term.set_cursor_position(position);
}

void redraw_line(const std::string& prompt, const std::string& line) {
    auto current_cursor {linebuffer.cursor_position()};

    term.erase_line();
    term.set_cursor_position({current_cursor.row, 0});
    term.write_text(prompt + line);
    term.set_cursor_position(current_cursor);
}

inline void move_cursor_right() {
    if (linebuffer.move_cursor_right()) {
        term.move_cursor_right();
    }
}

inline void move_cursor_left() {
    if (linebuffer.move_cursor_left()) {
        term.move_cursor_left();
    }
}

void erase_to_beginning() {
    if(linebuffer.erase_to_beginning()) {
        term.erase_to_line_start();
        redraw_line(_prompt, linebuffer.get_text());
    }
}

void erase_to_end() {
    if (linebuffer.erase_to_end())
        term.erase_to_line_end();
}

void erase_forward() {
    if (linebuffer.erase_forward())
        redraw_line(_prompt, linebuffer.get_text());
}

void erase_backwards() {
    if (linebuffer.erase_backwards()) {
        move_cursor_left();
        redraw_line(_prompt, linebuffer.get_text());
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

void go_to_line_start() {
    linebuffer.go_to_line_start();
    term.set_cursor_position(linebuffer.cursor_position());
}

void go_to_line_end() {
    linebuffer.go_to_line_end();
    term.set_cursor_position(linebuffer.cursor_position());
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
    {packn<key_code_t>(0, 0x43, 0x5b, 0x1b), move_cursor_right},
    {packn<key_code_t>(0, 0x44, 0x5b, 0x1b), move_cursor_left},
};

void init_readline(std::string_view prompt) {
    linebuffer.set_text("");
    _prompt = prompt;
    term.enable_raw_mode();
    linebuffer.line_start(_prompt.size() + 1);
    std::cout << "\r\n" << _prompt << std::flush;
    linebuffer.cursor_position(term.query_cursor_position());
}

void handle_control(key_code_t key) {
    try {
        command_map.at(key)();
    } catch (const std::out_of_range& e) {
        // Not implemented. Do nothing?
    }
}

void handle_normal(key_code_t key) {
    linebuffer.insert(key);
    redraw_line(_prompt, linebuffer.get_text());
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
    return linebuffer.get_text();
}
