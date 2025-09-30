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
    std::stack<state> states;
    std::string_view line;
    std::string_view delimeter;
    args_container tokens;

    size_t token_start;
    size_t token_end;

    bool is_delimeter(char c) const;
    /* Advances token_start until a character that is not delimerter is met,
    * then assigns token_end to token_start */
    void find_start();
    /* Advances token_end n positions */
    void advance(int n = 1);
    bool is_next(char c) const;
    /* Tries to push a substring from token_start to token_end to the
     * resulting collection. Assigns token_start to token_end and advances
     * token_end on success*/
    void push_token();
    /* Handles char depending on current state. Returns true if the current
     * token has ended and is ready to be pushed, false otherwise. */
    bool handle_char(char c);

    /* Splits a string into tokens by characters provided in delimeter. */
    [[nodiscard]]
    args_container tokenize_impl();

    tokenizer(std::string_view line, std::string_view delimeter);

public:
    [[nodiscard]]
    static args_container tokenize(std::string_view line, std::string_view delimeter) {
        tokenizer t {line, delimeter};
        return t.tokenize_impl();
    }
};

