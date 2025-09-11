#pragma once

#include "cmd/cmd.h"
#include <string>

const char VAR_PREFIX = '$';
const char ESCAPE_CHAR = '\\';

[[nodiscard]]
std::string get_variable(const std::string& str) noexcept;

void expand_all_variables(std::string& str);

void expand_tilde(std::string& str);

void expand_globs(args_container& args);
