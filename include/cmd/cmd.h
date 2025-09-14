#pragma once

#include <span>
#include <string_view>
#include <vector>
#include <string>

using args_container = std::vector<std::string>;
using args_view = std::span<std::string>;

namespace sep {

constexpr std::string_view NEWLINE {"\n"};
constexpr std::string_view COMMAND {";"};
constexpr std::string_view LIST_AND {"&&"};
constexpr std::string_view LIST_OR {"||"};
constexpr std::string_view PIPE_OUT {"|"};
constexpr std::string_view PIPE_BOTH {"|&"};

} // namespace sep

int run_simple_command(args_view args);

int run_pipeline(args_view args);

int run_list(args_view args);

int run_compound_command(args_container& args);
