#include "parser.h"
#include "cmd/cmd.h"
#include "stringsep.h"
#include <string_view>

tokenizer::tokenizer(std::string_view line, std::string_view delimeter) :
    states({state::REGULAR}),
    token_start(0),
    token_end(0),
    line(line),
    delimeter(delimeter)
{}

args_container tokenizer::tokenize_impl() {
    do {
        find_start();
        if (token_start == std::string::npos)
            break;

        while (token_end < line.size() && !handle_char(line[token_end]));

        push_token();
    } while (token_start <= line.size() && token_end <= line.size());

    return tokens;
}

void tokenizer::find_start() {
    token_start = line.find_first_not_of(delimeter, token_start);
    token_end = token_start;
}

void tokenizer::push_token() {
    assert(token_end <= line.size());

    if (token_start < token_end) {
        tokens.push_back(std::string(line.substr(token_start, token_end - token_start)));
        token_start = token_end;
        token_end++;
    }
}

bool tokenizer::handle_char(char c) {
    assert(!states.empty());
    switch (states.top()) {
        case state::ESCAPED: {
            advance();
            states.pop();
            break;
        }
        case state::REGULAR: {
            if (is_delimeter(c))
                return true;

            switch (c) {
                case '\'': {
                    states.push(state::SINGLE_QUOTES);
                    advance();
                    break;
                }
                case '"': {
                    states.push(state::DOUBLE_QUOTES);
                    advance();
                    break;
                }
                case sep::ESCAPE_CHAR: {
                    states.push(state::ESCAPED);
                    advance();
                    break;
                }
                case '\n':
                case sep::COMMAND_CHAR: {
                    // token parsing just begun
                    if (token_end == token_start) {
                        advance();
                    }
                    return true;
                }
                case sep::OR_CHAR: {
                    if (token_end == token_start) {
                        advance();
                        if (is_next(sep::OR_CHAR) || is_next(sep::AND_CHAR))
                            advance();
                    }
                    return true;
                }
                case sep::AND_CHAR: {
                    if (token_end == token_start) {
                        advance();
                        if (is_next(sep::AND_CHAR))
                            advance();
                    }
                    return true;
                }
                case sep::COMMENT_CHAR: {
                    push_token();
                    token_start = std::string::npos; //HACK: invalidate position
                    return true;
                }
                default: {
                    advance();
                }
            }
            break;
        }
        case state::SINGLE_QUOTES: {
            switch (c) {
                case '\'': {
                    states.pop();
                    advance();
                    break;
                }
                case '"': {
                    states.push(state::DOUBLE_QUOTES);
                    advance();
                    break;
                }
                case sep::ESCAPE_CHAR: {
                    states.push(state::ESCAPED);
                    advance();
                    break;
                }
                default: {
                    advance();
                }
            }
            break;
        }
        case state::DOUBLE_QUOTES: {
            switch (c) {
                case '\'': {
                    states.push(state::SINGLE_QUOTES);
                    advance();
                    break;
                }
                case '"': {
                    states.pop();
                    advance();
                    break;
                }
                case sep::ESCAPE_CHAR: {
                    states.push(state::ESCAPED);
                    advance();
                    break;
                }
                default: {
                    advance();
                }
            }
            break;
        }
    }
    return false;
}

bool tokenizer::is_delimeter(char c) {
    return delimeter.find(c) != std::string::npos;
}

void tokenizer::advance(int n) {
    token_end += n;
}

bool tokenizer::is_next(char c) {
    if (token_end >= line.size())
        return false;
    return line[token_end] == c;
}
