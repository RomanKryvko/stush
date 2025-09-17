#pragma once

#include <string_view>

namespace sep {

// Separators that can be encountered as word tokens
constexpr std::string_view NEWLINE {"\n"};
constexpr std::string_view COMMAND {";"};
constexpr std::string_view LIST_AND {"&&"};
constexpr std::string_view LIST_OR {"||"};
constexpr std::string_view PIPE_OUT {"|"};
constexpr std::string_view PIPE_BOTH {"|&"};

// Separators that can be encountered as characters inside a token
constexpr char VAR_PREFIX = '$';
constexpr char ESCAPE_CHAR = '\\';
constexpr char COMMENT_CHAR = '#';
constexpr char COMMAND_CHAR = ';';
constexpr char OR_CHAR = '|';
constexpr char AND_CHAR = '&';

constexpr std::string_view WORD_SEPARATORS {R"( \/$:;-+[]{}()'"?*)"};

} // namespace sep
