#pragma once

#include <string>
#include <vector>

using cmd_function_t = int(*)(const std::vector<std::string>&);

struct Command {
    cmd_function_t function;
    std::string doc;
};

const int BUILTIN_NOT_FOUND = 127;

void err_too_many_args(std::string_view command);

int com_help(const std::vector<std::string>& args);

int com_clear(const std::vector<std::string>& args);

int com_exit(const std::vector<std::string>& args);

int exec_builtin(const std::vector<std::string>& args);
