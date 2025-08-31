#include "linereader/terminal.h"
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

// We need this to be global state to ensure that we can disable raw mode on exit
termios orig_termios {};
bool is_in_raw_mode {false};

Terminal::Terminal() = default;

Terminal::~Terminal() {
    disable_raw_mode();
}

void _disable_raw_mode() {
    if (!is_in_raw_mode)
        return;

    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
    is_in_raw_mode = false;
}

void Terminal::disable_raw_mode() {
    _disable_raw_mode();
}

void Terminal::enable_raw_mode() {
    if (is_in_raw_mode)
        return;

    atexit(_disable_raw_mode);
    if(tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }
    termios raw { orig_termios };
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_lflag &= ~(ECHO | ICANON);
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
    is_in_raw_mode = true;
}

void Terminal::set_cursor_position(cursor_pos cursor) {
    buffer += "\x1b[" + std::to_string(cursor.row) + ";" + std::to_string(cursor.col) + "H";
}

void Terminal::move_cursor_up() {
    buffer += "\x1b[A";
}

void Terminal::move_cursor_down() {
    buffer += "\x1b[B";
}

void Terminal::move_cursor_left() {
    buffer += "\x1b[D";
}

void Terminal::move_cursor_right() {
    buffer += "\x1b[C";
}

void Terminal::erase_line() {
    buffer += "\x1b[2K";
}

void Terminal::erase_to_line_end() {
    buffer += "\x1b[K";
}

void Terminal::erase_to_line_start() {
    buffer += "\x1b[1K";
}

void Terminal::clear_to_screen_end() {
    buffer += "\x1b[J";
}

void Terminal::write_text(std::string_view text) {
    buffer += text;
}

void Terminal::commit() {
    if (!buffer.empty()) {
        write(STDOUT_FILENO, buffer.c_str(), buffer.size());
        buffer.clear();
    }
}
