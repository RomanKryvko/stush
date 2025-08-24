#include <parser.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sched.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
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

struct cursor_pos {
    cursor_pos() = default;
    cursor_pos(int row, int col) : row(row), col(col) {}
    int row {};
    int col {};
};

cursor_pos current_cursor {};

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

void set_cursor_position(cursor_pos position) {
    std::string cup_sequnce = "\x1b[" + std::to_string(position.row) + ";" + std::to_string(position.col) + "H";
    write(STDIN_FILENO, cup_sequnce.c_str(), cup_sequnce.size());
    current_cursor = position;
}

const char CTRL_A = CTRL('a');
const char CTRL_E = CTRL('e');
const char CTRL_U = CTRL('u');
const char CTRL_K = CTRL('k');
const char CTRL_H = CTRL('h');
const char BACKSPACE = 0x7f;
const char ENTER = 0xd;
const char ESC = 0x1b;

void redraw_line(std::string_view line) {
    cursor_pos old_cursor = current_cursor;
    write(STDIN_FILENO, "\x1b[2K", 4);
    set_cursor_position({current_cursor.row, 1});//TODO: return to prompt start
    std::cout << line << std::flush;
    set_cursor_position({old_cursor.row, old_cursor.col});
}

void move_cursor_right() {
    set_cursor_position({current_cursor.row, current_cursor.col + 1});
}

void move_cursor_left() {
    set_cursor_position({current_cursor.row, current_cursor.col - 1});
}

std::string sh_read_line() {
    enable_raw_mode();
    current_cursor = query_cursor_pos();
    std::string line_state;
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != ENTER) {
        if (iscntrl(c)) {
            //TODO: move all of this into a map
            switch (c) {
                case CTRL_U: {
                    write(STDIN_FILENO, "\x1b[1K", 4);
                    line_state.erase(0, current_cursor.col);
                    set_cursor_position({current_cursor.row, 1});
                    break;
                }
                case CTRL_K: {
                    line_state.erase(current_cursor.col);
                    write(STDIN_FILENO, "\x1b[K", 3);
                    break;
                }
                case CTRL_A: {
                    set_cursor_position({current_cursor.row, 0});
                    break;
                }
                case CTRL_E: {
                    set_cursor_position({current_cursor.row, static_cast<int>(line_state.size())}); //FIXME: cursor always ends up ON the last character
                    break;
                }
                case BACKSPACE:
                case CTRL_H: {
                    if (!line_state.empty()) {
                        if (current_cursor.col > 1 && current_cursor.col < line_state.size() - 2) {
                            line_state.erase(current_cursor.col, 1);
                        } else {
                            line_state.pop_back();
                        }
                        move_cursor_left();
                        redraw_line(line_state);
                    }
                    break;
                }
                case ESC: {
                    read(STDIN_FILENO, &c, 1);
                    //TODO: read the entire sequence and use sscanf here?
                    if (c == '[') {
                        read(STDIN_FILENO, &c, 1);
                        char seq[4] {'\x1b', '[', c, '\0'}; //TODO: handle CTRL+arrow
                        write(STDIN_FILENO, &seq, 4);
                    }
                    break;
                }
            }
        } else {
            if (current_cursor.col < line_state.size()) { //TODO: line_state + prompt_length
                line_state.insert(current_cursor.col, 1, c);
            } else {
                line_state += c;
            }
            redraw_line(line_state);
            move_cursor_right();
        }
    }
    disable_raw_mode();
    return line_state;
}

//NOTE: we have to use std::string because exec expects a null-terminated string
int sh_execute(std::vector<std::string> args) {
    const char* args_c [args.size() + 1];
    for (int i = 0; i < args.size(); i++) {
        args_c[i] = args[i].c_str();
    }
    args_c[args.size()] = nullptr;

    int status {};
    pid_t pid = fork();
    if (!pid) {
        if(execvp(args[0].c_str(), const_cast<char**>(args_c)) == -1) {
            perror("execvp");
        }
        exit(EXIT_FAILURE);
    } else if (pid == -1) {
        perror("fork");
    } else {
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return WEXITSTATUS(status);
}

int sh_main_loop(int argc, const char** argv) {
    char delimeter = ' ';
    while (true) {
        // TODO: make prompt variable
        // std::cout << "> ";
        std::string line = sh_read_line();
        if (line.empty())
            continue;
        std::vector<std::string> args = sh_tokenize(line, delimeter);
        std::cout << "\n";
        int status = sh_execute(args);
        std::cout << "\nProcess " << args[0] << " exited with code " << status << '\n';
    }
}

int main(int argc, char** argv) {
    sh_main_loop(argc, (const char**) argv);
}
