#include "parser.h"
#include "cmd/cmd.h"
#include <cstddef>
#include <stdexcept>
#include <string_view>

inline bool is_com_sep(char c) {
    return c == ';';
}

inline bool is_list_sep(char c) {
    return c == '&' || c == '|';
}

constexpr bool is_escaped(std::string_view str, size_t pos) {
    return pos > 0 && str[pos - 1] == '\\';
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

        while (pos_start < line.size() && line[pos_start] == delimeter && !is_escaped(line, pos_start)) {
            pos_start++;
        }
        if (pos_start == line.size())
            break;

        pos_end = pos_start;
        while (pos_end < line.size() && (line[pos_end] != delimeter || is_escaped(line, pos_end))) {
            if (line[pos_end] == '\'' && !in_dquotes && !is_escaped(line, pos_end)) {
                in_squotes = !in_squotes;
            } else if (line[pos_end] == '\"' && !in_squotes && !is_escaped(line, pos_end)) {
                in_dquotes = !in_dquotes;
            } else if (is_com_sep(line[pos_end]) || is_list_sep(line[pos_end])) {
                if (pos_start != pos_end)
                    res.push_back(std::string(line.substr(pos_start, pos_end - pos_start)));
                pos_start = pos_end;
                pos_end++;
                if (pos_end < line.size() && is_list_sep(line[pos_end]) && line[pos_end - 1] == line[pos_end]) {
                    pos_end++;
                }
                res.push_back(std::string(line.substr(pos_start, pos_end - pos_start)));
                pos_start = pos_end;
                continue;
            }
            pos_end++;
        }
        if (pos_start == pos_end)
            continue;

        if (pos_end == line.size()) {
            if (in_squotes || in_dquotes) {
                throw std::runtime_error("Quote marks mismatch");
            }

            res.push_back(std::string(line.substr(pos_start)));
            break;
        }

        if (in_squotes || in_dquotes) {
            const char quote {in_squotes ? '\'' : '\"'};
            pos_end = line.find(quote, pos_end);
            if (pos_end == std::string_view::npos)
                throw std::runtime_error("Quote marks mismatch");
            pos_end++;
        }

        res.push_back(std::string(line.substr(pos_start, pos_end - pos_start)));

        pos_start = pos_end + 1;
    } while (pos_start < line.size());

    return res;
}
