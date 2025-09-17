#pragma once

#include <span>
#include <vector>
#include <string>

using args_container = std::vector<std::string>;
using args_view = std::span<std::string>;

int run_simple_command(args_view args);

int run_pipeline(args_view args);

int run_list(args_view args);

int run_compound_command(args_container& args);
