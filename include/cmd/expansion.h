#pragma once

#include "cmd/cmd.h"
#include <string>

const char VAR_PREFIX = '$';
const char ESCAPE_CHAR = '\\';

[[nodiscard]]
std::string get_variable(const std::string& str) noexcept;

/* Expand shell and env variables, tilde, strip escaping slashes. */
void expand_word(std::string& str);

void expand_tilde(std::string& str);

void expand_globs(args_container& args);

void strip_all_quotes(args_view args);

void strip_quotes(std::string& str);
