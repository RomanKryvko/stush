#pragma once

#include "cmd/cmd.h"
#include <string>

[[nodiscard]]
std::string get_variable(const std::string& str) noexcept;

/* Expand shell and env variables, tilde, strip escaping slashes. */
void expand_word(std::string& str);

void expand_globs(args_container& args);

void strip_all_quotes(args_view args);
