#include "builtins/builtins.h"
#include "cmd/cmd.h"
#include "cmd/expansion.h"
#include <cstdlib>
#include <stdexcept>
#include <string_view>
#include <unistd.h>
#include <wait.h>

int run_simple_command_impl(args_view args) {
    int builtin_status {exec_builtin(args)};
    if (builtin_status != BUILTIN_NOT_FOUND)
        return builtin_status;

    const char* args_c [args.size() + 1];
    for (int i = 0; i < args.size(); i++) {
        args_c[i] = args[i].c_str();
    }
    args_c[args.size()] = nullptr;

    int status {};
    pid_t pid {fork()};
    if (!pid) {
        if (execvp(args[0].c_str(), const_cast<char**>(args_c)) == -1) {
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

int run_simple_command(args_view args) {
    bool has_wildcard {false};

    for (auto& arg : args) {
        expand_all_variables(arg);
        expand_tilde(arg);
        if (arg.find('*') != std::string::npos)
            has_wildcard = true;
    }

    if (has_wildcard) {
        args_container copy;
        copy.assign(args.begin(), args.end());
        expand_globs(copy);
        strip_all_quotes(copy);
        return run_simple_command_impl(copy);
    }
    strip_all_quotes(args);
    return run_simple_command_impl(args);
}

int run_pipeline(args_view args) {
    return run_simple_command(args); //FIXME: implement
}

struct list_item {
    args_view args;
    std::string_view separator; // TODO: use enum here?
};

list_item get_next_pipeline(args_view::iterator& it, const args_view::iterator end) {
    const auto start {it};
    while (it != end) {
        const std::string& s {*it};
        if (s == sep::LIST_AND || s == sep::LIST_OR) {
            const args_view command {start, it};
            if (command.empty()) {
                throw std::runtime_error("Missing command in list.");
            }

            it++;
            return {command, s};
        }
        it++;
    }
    return {{start, it}, ""};
}

int run_list(args_view args) {
    int status {};

    auto it {args.begin()};

    while (it != args.end()) {
        const auto command = get_next_pipeline(it, args.end());
        status = run_pipeline(command.args);

        if (command.separator == sep::LIST_AND && status != EXIT_SUCCESS) {
            return status;
        }
        if (command.separator == sep::LIST_OR && status == EXIT_SUCCESS) {
            return status;
        }
    }

    return status;
}

std::vector<args_view> split_compound_command(args_container& args) {
    std::vector<args_view> res {};

    auto prev {args.begin()};
    for (auto it = args.begin(); it != args.end(); it++) {
        const std::string& s {*it};
        if (s == sep::NEWLINE || s == sep::COMMAND) {
            const args_view command {prev, it};
            if (!command.empty()) {
                res.push_back(command);
            }
            prev = ++it;
        }
    }

    if (res.empty()) { //NOTE: probably redundant, as all commands contain at least one newline
        res.push_back(args);
    }
    return res;
}

int run_compound_command(args_container& args) {
    int status {};
    for (auto command : split_compound_command(args)) {
        status = run_list(command);
    }
    return status;
}
