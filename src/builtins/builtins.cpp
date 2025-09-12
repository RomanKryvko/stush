#include "builtins/builtins.h"
#include "builtins/cd.h"
#include "cmd/cmd.h"
#include "cmd/variable.h"
#include "linereader/terminal.h"
#include <cstdio>
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
    {"set", {com_set, "Set a shell variable"}},
    {"export", {com_export, "Set an environment variable"}},
    {"unset", {com_unset, "Unset a variable"}},
};

void err_too_many_args(std::string_view command) {
    std::cerr << command << ": too many arguments" << '\n';
}

int com_help(args_view args) {
    for (const auto& [k,v] : commands) {
        std::cout << k << ": " << v.doc << '\n';
    }
    return EXIT_SUCCESS;
}

int com_clear(args_view args) {
    Terminal term {};
    term.set_cursor_position({1, 1});
    term.clear_to_screen_end();
    term.commit();
    return EXIT_SUCCESS;
}

int com_exit(args_view args) {
    if (args.size() == 1)
        exit(EXIT_SUCCESS);

    if (args.size() > 2) {
        err_too_many_args(args[0]);
        return EXIT_FAILURE;
    }

    exit(std::stoi(args[1]));
}

int com_set(args_view args) {
    switch (args.size()) {
        case 1: {
            //TODO: print all shell vars?
            return EXIT_SUCCESS;
        }
        case 2: {
            var::set_var(args[1].c_str(), "");
            return EXIT_SUCCESS;
        }
        case 3: {
            var::set_var(args[1].c_str(), args[2].c_str());
            return EXIT_SUCCESS;
        }
    }
    err_too_many_args(args[0]);
    return EXIT_FAILURE;
}

int com_export(args_view args) {
    switch (args.size()) {
        case 1: {
            //TODO: print all env vars?
            return EXIT_SUCCESS;
        }
        case 2: {
            if (setenv(args[1].c_str(), "", 1) == -1) {
                perror("setenv");
                return EXIT_FAILURE;
            }
            return EXIT_SUCCESS;
        }
        case 3: {
            if (setenv(args[1].c_str(), args[2].c_str(), 1) == -1) {
                perror("setenv");
                return EXIT_FAILURE;
            }
            return EXIT_SUCCESS;
        }
    }
    err_too_many_args(args[0]);
    return EXIT_FAILURE;
}

int com_unset(args_view args) {
    if (args.size() == 1) {
        std::cerr << "Variable to unset not provided.\n";
        return EXIT_FAILURE;
    }

    if (args.size() > 2) {
        err_too_many_args(args[0]);
        return EXIT_FAILURE;
    }

    if (var::is_set(args[1])) {
        var::unset(args[1]);
        return EXIT_SUCCESS;
    }

    if (unsetenv(args[1].c_str()) == -1) {
        perror("unsetenv");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int exec_builtin(args_view args) {
    cmd_function_t builtin {};
    try {
        builtin = commands.at(args[0]).function;
    } catch (const std::out_of_range& e) {
        return BUILTIN_NOT_FOUND;
    }
    return builtin(args);
}
