#pragma once

#include "linereader.h"
#include <string>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <stdexcept>

class Terminal {
private:
    termios orig_termios {};
    bool is_in_raw_mode {false};
    std::string buffer {};

public:
    Terminal();
    ~Terminal();

    static key_code_t read_key() {
        key_code_t key {};
        if (read(STDIN_FILENO, &key, sizeof key) != -1) {
            return key;
        }
        throw std::runtime_error("Error reading key"); //TODO: create a separate exception class
    }

    static cursor_pos query_cursor_position() {
        write(STDOUT_FILENO, "\x1b[6n", 4);

        const int buf_size = 32;
        char buf[buf_size];
        int i = 0;
        char c;
        while (i < buf_size - 1) {
            if (read(STDIN_FILENO, &c, 1) != 1)
                break;
            buf[i++] = c;
            if (c == 'R')
                break;
        }
        buf[i] = '\0';

        // Response format: ESC [ row ; col R
        cursor_pos position;
        if (sscanf(buf, "\x1b[%d;%dR", &position.row, &position.col) == 2)
            return position;

        //TODO: introduce some better error handling
        throw std::runtime_error("Failed to read cursor position: " + std::string(buf));
    }

    void set_cursor_position(const cursor_pos& cursor);

    void disable_raw_mode();
    void enable_raw_mode();

    void move_cursor_up();
    void move_cursor_down();
    void move_cursor_left();
    void move_cursor_right();

    void erase_line();
    void erase_to_line_end();
    void erase_to_line_start();

    void write_text(std::string_view text);
    void commit();
};
