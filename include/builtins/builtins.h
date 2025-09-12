#pragma once

#include <string>
#include "cmd/cmd.h"

using cmd_function_t = int(*)(args_view);

struct Command {
    cmd_function_t function;
    std::string doc;
};

const int BUILTIN_NOT_FOUND = 127;

void err_too_many_args(std::string_view command);

int com_help(args_view args);

int com_clear(args_view args);

int com_exit(args_view args);

int com_set(args_view args);

int com_export(args_view args);

int com_unset(args_view args);

int exec_builtin(args_view args);
