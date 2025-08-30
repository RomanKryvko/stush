#include "linereader/linereader.h"
#include "linereader/types.h"
#include <iostream>

void LineReader::set_cursor_position(cursor_pos position) {
    linebuffer.cursor_position(position);
    term.set_cursor_position(position);
}

void LineReader::redraw_line(const std::string& prompt, const std::string& line) {
    auto current_cursor {linebuffer.cursor_position()};

    term.erase_line();
    term.set_cursor_position({current_cursor.row, 0});
    term.write_text(prompt + line);
    term.set_cursor_position(current_cursor);
}

void LineReader::move_cursor_right() {
    if (linebuffer.move_cursor_right()) {
        term.move_cursor_right();
    }
}

void LineReader::move_cursor_left() {
    if (linebuffer.move_cursor_left()) {
        term.move_cursor_left();
    }
}

void LineReader::erase_to_beginning() {
    if(linebuffer.erase_to_beginning()) {
        term.erase_to_line_start();
        redraw_line(_prompt, linebuffer.get_text());
    }
}

void LineReader::erase_to_end() {
    if (linebuffer.erase_to_end())
        term.erase_to_line_end();
}

void LineReader::erase_forward() {
    if (linebuffer.erase_forward())
        redraw_line(_prompt, linebuffer.get_text());
}

void LineReader::erase_backwards() {
    if (linebuffer.erase_backwards()) {
        move_cursor_left();
        redraw_line(_prompt, linebuffer.get_text());
    }
}

void LineReader::cursor_up() {
    // Do nothing.
    // TODO: implement history lookup here
}

void LineReader::cursor_down() {
    // Do nothing.
    // TODO: implement history lookup here
}

void LineReader::clear() {
    term.set_cursor_position({1, 1});
    term.clear_to_screen_end();
    const int col {linebuffer.cursor_position().col};
    set_cursor_position({1, col});
    redraw_line(_prompt, linebuffer.get_text());
}

void LineReader::go_to_line_start() {
    linebuffer.go_to_line_start();
    term.set_cursor_position(linebuffer.cursor_position());
}

void LineReader::go_to_line_end() {
    linebuffer.go_to_line_end();
    term.set_cursor_position(linebuffer.cursor_position());
}

void LineReader::init_readline(std::string_view prompt) {
    linebuffer.set_text("");
    _prompt = prompt;
    term.enable_raw_mode();
    linebuffer.line_start(_prompt.size() + 1);
    std::cout << "\r\n" << _prompt << std::flush;
    linebuffer.cursor_position(term.query_cursor_position());
}

void LineReader::handle_control(key_code_t key) {
    try {
        command_map.at(key)();
    } catch (const std::out_of_range& e) {
        // Not implemented. Do nothing?
    }
}

void LineReader::handle_normal(key_code_t key) {
    linebuffer.insert(key);
    redraw_line(_prompt, linebuffer.get_text());
    move_cursor_right();
}

std::string LineReader::sh_read_line(std::string_view prompt, char terminator) {
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
