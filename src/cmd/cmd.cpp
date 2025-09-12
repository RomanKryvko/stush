#include "builtins/builtins.h"
#include "cmd/cmd.h"
#include "cmd/expansion.h"
#include <unistd.h>
#include <wait.h>

int run_simple_command_impl(const args_container& args) {
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

int run_simple_command(const args_container& args) {
    for (const auto& word : args) {
        if (word.find('*') != std::string::npos) {
            auto copy {args};
            for (auto& arg : copy) {
                expand_all_variables(arg);
                expand_tilde(arg);
            }
            expand_globs(copy);
            return run_simple_command_impl(copy);
        }
    }
    return run_simple_command_impl(args);
}

