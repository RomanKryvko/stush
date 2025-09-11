#pragma once

#include <string>
#include <vector>

using args_container = std::vector<std::string>;

const char VAR_PREFIX = '$';
const char ESCAPE_CHAR = '\\';

[[nodiscard]]
std::string get_variable(const std::string& str) noexcept;

void expand_all_variables(std::string& str);

void expand_tilde(std::string& str);
