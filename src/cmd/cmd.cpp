#include "builtins/builtins.h"
#include "cmd/cmd.h"
#include "cmd/expansion.h"
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
        return run_simple_command_impl(copy);
    }
    return run_simple_command_impl(args);
}

