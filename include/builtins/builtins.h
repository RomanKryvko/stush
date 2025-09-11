#pragma once

#include <string>
#include "cmd/cmd.h"

using cmd_function_t = int(*)(const args_container&);

struct Command {
    cmd_function_t function;
    std::string doc;
};

const int BUILTIN_NOT_FOUND = 127;

void err_too_many_args(std::string_view command);

int com_help(const args_container& args);

int com_clear(const args_container& args);

int com_exit(const args_container& args);

int exec_builtin(const args_container& args);
