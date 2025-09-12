#include "parser.h"
#include "cmd/cmd.h"
#include <cstddef>
#include <stdexcept>
#include <string_view>

constexpr inline bool is_quote(char c) {
    return c == '\'' || c == '\"';
}

args_container sh_tokenize(std::string_view line, char delimeter) {
    args_container res {};
    if (line.empty()) {
        return res;
    }

    size_t pos_start {};
    size_t pos_end {};
    do {
        bool in_squotes {false};
        bool in_dquotes {false};

        while (pos_start < line.size() && line[pos_start] == delimeter) {
            pos_start++;
        }
        if (pos_start == line.size())
            break;

        pos_end = pos_start;
        while (pos_end < line.size() && line[pos_end] != delimeter) {
            if (line[pos_end] == '\'' && !in_dquotes) {
                in_squotes = !in_squotes;
            } else if (line[pos_end] == '\"' && !in_squotes) {
                in_dquotes = !in_dquotes;
            }
            pos_end++;
        }
        if (pos_end == line.size()) {
            if (in_squotes || in_dquotes) {
                throw std::runtime_error("Quote marks mismatch");
            }

            if (is_quote(line[pos_start]) && is_quote(line[pos_end - 1])) {
                res.push_back(std::string(line.substr(pos_start + 1, pos_end - pos_start - 2)));
            } else {
                res.push_back(std::string(line.substr(pos_start)));
            }
            break;
        }

        if (in_squotes || in_dquotes) {
            pos_start++; // Skip the opening quote
            const char quote {in_squotes ? '\'' : '\"'};
            pos_end = line.find(quote, pos_end);
            if (pos_end == std::string_view::npos)
                throw std::runtime_error("Quote marks mismatch");
        }

        if (is_quote(line[pos_start]) && is_quote(line[pos_end - 1])) {
            res.push_back(std::string(line.substr(pos_start + 1, pos_end - pos_start - 2)));
        } else {
            res.push_back(std::string(line.substr(pos_start, pos_end - pos_start)));
        }

        pos_start = pos_end + 1;
    } while (pos_start < line.size());

    return res;
}
