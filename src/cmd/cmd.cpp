#include "builtins/builtins.h"
#include "cmd/cmd.h"
#include "cmd/expansion.h"
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <sched.h>
#include <stdexcept>
#include <string_view>
#include <unistd.h>
#include <wait.h>

void run_process(args_view args) {
    const char* argv[args.size() + 1];
    for (size_t i = 0; i < args.size(); i++) {
        argv[i] = args[i].c_str();
    }
    argv[args.size()] = nullptr;

    execvp(argv[0], const_cast<char**>(argv));
    perror("execvp");
    exit(EXIT_FAILURE);
}

/* Run a simple command or a shell builtin. Assumes that all expansions
 * of variables, globs, etc. have already been done*/
int run_simple_command(args_view args) {
    int builtin_status {exec_builtin(args)};
    if (builtin_status != BUILTIN_NOT_FOUND)
        return builtin_status;

    int status {};
    const pid_t pid {fork()};
    if (!pid) {
        run_process(args);
    } else if (pid == -1) {
        perror("fork");
    } else {
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return WEXITSTATUS(status);
}

/* Perform variable, tilde, glob expansion and strip quotes. */
args_container prepare_command_args(args_view args) {
    args_container result {args.begin(), args.end()};
    for (auto& arg : result) {
        expand_all_variables(arg);
        expand_tilde(arg);
    }
    expand_globs(result);
    strip_all_quotes(result);
    return result;
}

struct pipeline_item {
    args_container args;
    std::string_view separator; //TODO: use enum here?
};

/* Splits the pipeline by | and |& operators and performs expansions on each
 * of the resulting commands */
std::vector<pipeline_item> split_pipeline(args_view args) {
    std::vector<pipeline_item> res {};

    auto prev {args.begin()};
    for (auto it = args.begin(); it != args.end(); it++) {
        const std::string_view& s {*it};
        if (s == sep::PIPE_OUT || s == sep::PIPE_BOTH) {
            if (prev == it) {
                throw std::runtime_error("Missing command in pipeline.");
            }
            res.push_back({prepare_command_args({prev, it}), s});
            prev = it + 1;
        }
    }

    if (prev != args.end())
        res.push_back({prepare_command_args({prev, args.end()}), ""});

    return res;
}

inline void close_pipes(const std::vector<int[2]>& pipes) {
    for (auto pipe : pipes) {
        close(pipe[0]);
        close(pipe[1]);
    }
}

int run_pipeline(args_view args) {
    auto pipelines {split_pipeline(args)};
    assert(!pipelines.empty());

    const size_t n {pipelines.size()};
    if (n == 1)
        return run_simple_command(pipelines[0].args);

    std::vector<int[2]> pipes(n - 1);
    for (auto p : pipes) {
        if (pipe(p) == -1) {
            perror("pipe");
            return EXIT_FAILURE;
        }
    }

    std::vector<pid_t> children (n);
    //NOTE: this is ugly, but it allows us to avoid ifs
    {
        const pid_t pid {fork()};
        if (!pid) {
            dup2(pipes[0][1], STDOUT_FILENO);

            close_pipes(pipes);

            //TODO: implement running bultins here
            run_process(pipelines[0].args);
        } else if (pid < 0) {
            perror("fork");
            return EXIT_FAILURE;
        }
        children[0] = pid;
    }
    for (size_t i = 1; i < n - 1; i++) {
        const pid_t pid {fork()};
        if (!pid) {
            dup2(pipes[i - 1][0], STDIN_FILENO);
            dup2(pipes[i][1], STDOUT_FILENO);

            close_pipes(pipes);

            run_process(pipelines[i].args);
        } else if (pid < 0) {
            perror("fork");
            return EXIT_FAILURE;
        }
        children[i] = pid;
    }
    {
        const auto last {n - 1};
        const pid_t pid {fork()};
        if (!pid) {
            dup2(pipes[last - 1][0], STDIN_FILENO);

            close_pipes(pipes);

            run_process(pipelines[last].args);
        } else if (pid < 0) {
            perror("fork");
            return EXIT_FAILURE;
        }
        children[last] = pid;
    }

    close_pipes(pipes);

    int pl_status {};
    for (pid_t pid : children) {
        int status {};
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        pl_status = status;
    }

    return WEXITSTATUS(pl_status);
}

struct list_item {
    args_view args;
    std::string_view separator; // TODO: use enum here?
};

/* Gets the next pipeline from list and advances the argument iterator. */
list_item get_next_pipeline(args_view::iterator& it, const args_view::iterator end) {
    const auto start {it};
    while (it != end) {
        const std::string_view& s {*it};
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
        const std::string_view& s {*it};
        if (s == sep::NEWLINE || s == sep::COMMAND) {
            const args_view command {prev, it};
            if (!command.empty()) {
                res.push_back(command);
            }
            prev = it + 1;
        }
    }

    if (prev != args.end())
        res.push_back({prev, args.end()});

    return res;
}

int run_compound_command(args_container& args) {
    int status {};
    for (auto command : split_compound_command(args)) {
        status = run_list(command);
    }
    return status;
}
