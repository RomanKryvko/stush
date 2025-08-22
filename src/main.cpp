#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sched.h>
#include <string>
#include <string_view>
#include <vector>
#include <unistd.h>
#include <wait.h>

std::string sh_read_line() {
    std::string line {};
    std::getline(std::cin, line);
    return line;
}

std::vector<std::string> sh_tokenize(std::string_view line, char delimeter) {
    std::vector<std::string> res {};
    if (line.empty()) {
        return res;
    }

    size_t pos_start {};
    size_t pos_end {};
    do {
        pos_start = line.find_first_not_of(delimeter, pos_start);
        if (pos_start == std::string_view::npos) {
            break;
        }
        std::string token {};
        pos_end = line.find(delimeter, pos_start);
        if (pos_end != std::string_view::npos) {
            token = line.substr(pos_start, pos_end - pos_start);
        } else {
            token = line.substr(pos_start);
        }
        res.push_back(token);
        pos_start = pos_end + 1;
    } while (pos_end != std::string_view::npos);

    return res;
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
        std::cout << "> ";
        std::string line = sh_read_line();
        if (line.empty())
            continue;
        std::vector<std::string> args = sh_tokenize(line, delimeter);
        int status = sh_execute(args);
        std::cout << "Process " << args[0] << " exited with code " << status << '\n';
    }
}

int main(int argc, char** argv) {
    sh_main_loop(argc, (const char**) argv);
}
