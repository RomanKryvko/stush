#pragma once

#include "cmd/cmd.h"
#include <cassert>
#include <cstddef>
#include <stack>
#include <string_view>

class tokenizer {
    enum class state {
        REGULAR,
        ESCAPED,
        SINGLE_QUOTES,
        DOUBLE_QUOTES,
    };
    std::stack<state> states {{state::REGULAR}};
    std::string_view m_line;
    std::string_view m_delimeter;
    args_container tokens;

    size_t token_start {};
    size_t token_end {};

    bool is_delimeter(char c);
    /* Advances token_start until a character that is not delimerter is met,
    * then assigns token_end to token_start */
    void find_start();
    /* Advances token_end n positions */
    void advance(int n = 1);
    bool is_next(char c);
    /* Tries to push a substring from token_start to token_end to the
     * resulting collection. Assigns token_start to token_end and advances
     * token_end on success*/
    void push_token();
    /* Handles char depending on current state. Returns true if the current
     * token has ended and is ready to be pushed, false otherwise. */
    bool handle_char(char c);

public:
    [[nodiscard]]
    args_container tokenize(std::string_view line, std::string_view delimeter);
};

args_container sh_tokenize(std::string_view line, std::string_view delimeter);
