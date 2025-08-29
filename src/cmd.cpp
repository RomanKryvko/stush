#include <cmd.h>
#include <byteutils.h>
#include <parser.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
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

termios orig_termios;
void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode() {
    if(tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }
    atexit(disable_raw_mode); //TODO: check whether we really need it
    termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_lflag &= ~(ECHO | ICANON);
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }
}

cursor_pos current_cursor {};

// starting column position
int line_start = 1;

std::string buffer {};
std::string line_state {};
std::string _prompt {};

cursor_pos query_cursor_pos() {
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

[[nodiscard]]
std::string set_cursor_position(cursor_pos position) {
    current_cursor = position;
    return "\x1b[" + std::to_string(position.row) + ";" + std::to_string(position.col) + "H";
}

[[nodiscard]]
std::string redraw_line(std::string_view prompt, std::string_view line) {
    return "\x1b[2K\x1b[" + std::to_string(current_cursor.row) + ";0H" +
            std::string(prompt) + std::string(line)
            + set_cursor_position({current_cursor.row, current_cursor.col});
}

[[nodiscard]]
inline std::string move_cursor_right() {
    return set_cursor_position({current_cursor.row, current_cursor.col + 1});
}

[[nodiscard]]
inline std::string move_cursor_left() {
    return set_cursor_position({current_cursor.row, current_cursor.col - 1});
}

int full_line_length() {
    return line_state.size() + line_start;
}

void go_to_line_start() {
    buffer += set_cursor_position({current_cursor.row, line_start});
}

void go_to_line_end() {
    buffer += set_cursor_position({current_cursor.row, static_cast<int>(line_state.size()) + line_start});
}

void erase_to_beginning() {
    line_state.erase(0, current_cursor.col - line_start);
    buffer += "\x1b[1K";
    buffer += set_cursor_position({current_cursor.row, line_start});
    buffer += redraw_line(_prompt, line_state);
}

void erase_to_end() {
    if (full_line_length() >= current_cursor.col) {
        line_state.erase(current_cursor.col - line_start);
        buffer += "\x1b[K";
    }
}

void erase_forward() {
    if (!line_state.empty() &&
        current_cursor.col > line_start &&
        current_cursor.col < full_line_length())
    {
        line_state.erase(current_cursor.col - line_start, 1);
        buffer += redraw_line(_prompt, line_state);
    }
}

void erase_backwards() {
    if (!line_state.empty() && current_cursor.col > line_start) {
        if (current_cursor.col < full_line_length()) {
            line_state.erase(current_cursor.col - line_start, 1);
        } else {
            line_state.pop_back();
        }
        buffer += move_cursor_left();
        buffer += redraw_line(_prompt, line_state);
    }
}

void cursor_up() {
    // Do nothing.
    // current_cursor.row++;
    // buffer += "\x1b[A";
    // TODO: implement history lookup here
}

void cursor_down() {
    // Do nothing.
    // current_cursor.row--;
    // buffer += "\x1b[B";
    // TODO: implement history lookup here
}

void cursor_right() {
    if (current_cursor.col < full_line_length()) {
        current_cursor.col++;
        buffer += "\x1b[C";
    }
}

void cursor_left() {
    if (current_cursor.col > line_start) {
        current_cursor.col--;
        buffer += "\x1b[D";
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
    enable_raw_mode();
    line_start = _prompt.size() + 1;
    std::cout << "\r\n" << _prompt << std::flush;
    current_cursor = query_cursor_pos();
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
    buffer += redraw_line(_prompt, line_state);
    buffer += move_cursor_right();
}

void commit_changes() {
    if (!buffer.empty()) {
        write(STDOUT_FILENO, buffer.c_str(), buffer.size());
        buffer.clear();
    }
}

std::string sh_read_line(std::string_view prompt, char terminator) {
    init_readline(prompt);
    key_code_t key {};
    while (read(STDIN_FILENO, &key, sizeof key) != -1 && key != terminator) {
        char lastchar = key & 0xFF;
        if (iscntrl(lastchar)) {
            handle_control(key);
        } else {
            handle_normal(key);
        }
        commit_changes();
        key = 0;
    }
    disable_raw_mode();
    return line_state;
}
