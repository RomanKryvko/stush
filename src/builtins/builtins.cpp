#include "builtins/builtins.h"
#include "builtins/cd.h"
#include "linereader/terminal.h"
#include <cstdlib>
#include <iostream>
#include <sched.h>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

std::unordered_map<std::string, Command> commands {
    {"cd", {com_cd, "Change directory"}},
    {"help", {com_help, "Print help message"}},
    {"clear", {com_clear, "Clear terminal screen"}},
    {"exit", {com_exit, "Exit shell with a code"}},
};

void err_too_many_args(std::string_view command) {
    std::cerr << command << ": too many arguments" << '\n';
}

int com_help(const std::vector<std::string>& args) {
    for (const auto& [k,v] : commands) {
        std::cout << k << ": " << v.doc << '\n';
    }
    return EXIT_SUCCESS;
}

int com_clear(const std::vector<std::string>& args) {
    Terminal term {};
    term.set_cursor_position({1, 1});
    term.clear_to_screen_end();
    term.commit();
    return EXIT_SUCCESS;
}

int com_exit(const std::vector<std::string>& args) {
    if (args.size() == 1)
        exit(EXIT_SUCCESS);

    if (args.size() > 2) {
        err_too_many_args(args[0]);
        return EXIT_FAILURE;
    }

    exit(std::stoi(args[1]));
}

int exec_builtin(const std::vector<std::string>& args) {
    cmd_function_t builtin {};
    try {
        builtin = commands.at(args[0]).function;
    } catch (const std::out_of_range& e) {
        return BUILTIN_NOT_FOUND;
    }
    return builtin(args);
}
