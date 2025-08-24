#include <cmd.h>
#include <parser.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sched.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unistd.h>
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

[[nodiscard ]]
std::string set_cursor_position(cursor_pos position) {
    current_cursor = position;
    return "\x1b[" + std::to_string(position.row) + ";" + std::to_string(position.col) + "H";
}

[[nodiscard ]]
std::string redraw_line(std::string_view prompt, std::string_view line) {
    return "\x1b[2K\x1b[" + std::to_string(current_cursor.row) + ";0H" +
            std::string(prompt) + std::string(line)
            + set_cursor_position({current_cursor.row, current_cursor.col});
}

[[nodiscard ]]
inline std::string move_cursor_right() {
    return set_cursor_position({current_cursor.row, current_cursor.col + 1});
}

[[nodiscard ]]
inline std::string move_cursor_left() {
    return set_cursor_position({current_cursor.row, current_cursor.col - 1});
}

std::string sh_read_line(std::string_view prompt) {
    enable_raw_mode();
    line_start = prompt.size() + 1;
    std::cout << "\r\n" << prompt << std::flush;
    current_cursor = query_cursor_pos();
    std::string line_state;
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != ENTER) {
        std::string buffer;
        if (iscntrl(c)) {
            //TODO: move all of this into a map
            switch (c) {
                case CTRL_U: {
                    line_state.erase(0, current_cursor.col - line_start);
                    buffer += "\x1b[1K";
                    buffer += set_cursor_position({current_cursor.row, line_start});
                    buffer += redraw_line(prompt, line_state);
                    break;
                }
                case CTRL_K: {
                    if (line_state.size() + line_start >= current_cursor.col) {
                        line_state.erase(current_cursor.col - line_start);
                        buffer += "\x1b[K";
                    }
                    break;
                }
                case CTRL_A: {
                    buffer += set_cursor_position({current_cursor.row, line_start});
                    break;
                }
                case CTRL_E: {
                    buffer += set_cursor_position({current_cursor.row, static_cast<int>(line_state.size()) + line_start});
                    break;
                }
                case BACKSPACE:
                case CTRL_H: {
                    if (!line_state.empty() && current_cursor.col > line_start) {
                        if (current_cursor.col < line_state.size() + line_start) {
                            line_state.erase(current_cursor.col - line_start, 1);
                        } else {
                            line_state.pop_back();
                        }
                        buffer += move_cursor_left();
                        buffer += redraw_line(prompt, line_state);
                    }
                    break;
                }
                case ESC: {
                    read(STDIN_FILENO, &c, 1);
                    //TODO: read the entire sequence and use sscanf here?
                    if (c == '[') {
                        read(STDIN_FILENO, &c, 1);
                        char seq[3] {'\x1b', '[', c}; //TODO: handle CTRL+arrow
                        switch (c) {
                            case 'A': {
                                current_cursor.row++;
                                buffer += seq;
                                break;
                            }
                            case 'B': {
                                current_cursor.row--;
                                buffer += seq;
                                break;
                            }
                            case 'C': {
                                current_cursor.col++;
                                buffer += seq;
                                break;
                            }
                            case 'D': {
                                if (current_cursor.col > line_start) {
                                    current_cursor.col--;
                                    buffer += seq;
                                }
                                break;
                            }
                            case '3': { // DEL keypress
                                read(STDIN_FILENO, &c, 1);
                                if (c == '~' && !line_state.empty() &&
                                    current_cursor.col > line_start &&
                                    current_cursor.col < line_state.size() + line_start)
                                {
                                    line_state.erase(current_cursor.col - line_start, 1);
                                    buffer += redraw_line(prompt, line_state);
                                }
                                break;
                            }
                        }
                    }
                    break;
                }
            }
        } else {
            if (current_cursor.col < line_state.size() + line_start) {
                line_state.insert(current_cursor.col - line_start, 1, c);
            } else {
                line_state += c;
            }
            buffer += redraw_line(prompt, line_state);
            buffer += move_cursor_right();
        }
        write(STDIN_FILENO, buffer.c_str(), buffer.size());
    }
    disable_raw_mode();
    return line_state;
}
