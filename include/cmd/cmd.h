#pragma once

#include <span>
#include <vector>
#include <string>

using args_container = std::vector<std::string>;
using args_view = std::span<std::string>;

int run_simple_command(args_view args);

std::vector<args_view> split_pipelines(const args_container& args);

std::vector<args_view> split_lists(const args_container& args);

std::vector<args_view> split_compound_command(const args_container& args);
