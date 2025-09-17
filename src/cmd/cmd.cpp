#include "builtins/builtins.h"
#include "cmd/cmd.h"
#include "cmd/expansion.h"
#include "stringsep.h"
#include <cassert>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <sched.h>
#include <stdexcept>
#include <string_view>
#include <unistd.h>
#include <wait.h>

inline int signal_status(int status) {
    return 128 + WTERMSIG(status);
}

static int wait_for_child(pid_t pid) {
    int status {};
    while (true) {
        waitpid(pid, &status, WUNTRACED);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        if (WIFSIGNALED(status)) {
            return signal_status(status);
        }
    }
}

static void run_process(args_view args) {
    signal(SIGINT, SIG_DFL);
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
    }
    return wait_for_child(pid);
}

/* Perform variable, tilde, glob expansion and strip quotes. */
static args_container prepare_command_args(args_view args) {
    args_container result {args.begin(), args.end()};
    for (auto& arg : result) {
        expand_word(arg);
    }
    expand_globs(result);
    strip_all_quotes(result);
    return result;
}

enum class pipe_type {
    PIPE_STDOUT,
    PIPE_BOTH,
};

struct pipeline_item {
    args_container args;
    pipe_type type;
};

/* Splits the pipeline by | and |& operators and performs expansions on each
 * of the resulting commands */
static std::vector<pipeline_item> split_pipeline(args_view args) {
    std::vector<pipeline_item> res {};

    auto prev {args.begin()};
    for (auto it = args.begin(); it != args.end(); it++) {
        const std::string_view& s {*it};
        if (s == sep::PIPE_OUT || s == sep::PIPE_BOTH) {
            if (prev == it) {
                throw std::runtime_error("Missing command in pipeline.");
            }
            const pipe_type type {s == sep::PIPE_BOTH ? pipe_type::PIPE_BOTH : pipe_type::PIPE_STDOUT};
            res.push_back({prepare_command_args({prev, it}), type});
            prev = it + 1;
        }
    }

    if (prev != args.end())
        res.push_back({prepare_command_args({prev, args.end()}), pipe_type::PIPE_STDOUT});

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

    const size_t ncommands {pipelines.size()};
    if (ncommands == 1)
        return run_simple_command(pipelines[0].args);

    const size_t npipes {ncommands - 1};
    std::vector<int[2]> pipes(npipes);
    for (auto p : pipes) {
        if (pipe(p) == -1) {
            perror("pipe");
            return EXIT_FAILURE;
        }
    }

    std::vector<pid_t> children (ncommands);
    for (size_t i = 0; i < ncommands; i++) {
        const pid_t pid {fork()};
        if (!pid) {
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }
            if (i < npipes) {
                dup2(pipes[i][1], STDOUT_FILENO);
                if (pipelines[i].type == pipe_type::PIPE_BOTH) {
                    dup2(pipes[i][1], STDERR_FILENO);
                }
            }

            close_pipes(pipes);

            int builtin_status {exec_builtin(pipelines[i].args)};
            if (builtin_status != BUILTIN_NOT_FOUND)
                exit(builtin_status);

            run_process(pipelines[i].args);
        } else if (pid < 0) {
            perror("fork");
            return EXIT_FAILURE;
        }
        children[i] = pid;
    }

    close_pipes(pipes);

    int pl_status {};
    for (pid_t pid : children) {
        pl_status = wait_for_child(pid);
    }

    return pl_status;
}

enum list_type {
    AND,
    OR,
};

struct list_item {
    args_view args;
    list_type type;
};

/* Gets the next pipeline from list and advances the argument iterator. */
static list_item get_next_pipeline(args_view args, args_view::iterator& it) {
    const auto start {it};
    while (it != args.end()) {
        const std::string_view& s {*it};
        if (s == sep::LIST_AND || s == sep::LIST_OR) {
            const args_view command {start, it};
            if (command.empty()) {
                throw std::runtime_error("Missing command in list.");
            }

            it++;
            const list_type type {s == sep::LIST_AND ? list_type::AND : list_type::OR};
            return {command, type};
        }
        it++;
    }
    return {{start, it}, list_type::AND};
}

int run_list(args_view args) {
    int status {};

    auto it {args.begin()};

    while (it != args.end()) {
        const auto command = get_next_pipeline(args, it);
        status = run_pipeline(command.args);

        if (command.type == list_type::AND && status != EXIT_SUCCESS) {
            return status;
        }
        if (command.type == list_type::OR && status == EXIT_SUCCESS) {
            return status;
        }
    }

    return status;
}

static std::vector<args_view> split_compound_command(args_container& args) {
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
