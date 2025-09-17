#include "parser.h"
#include "cmd/cmd.h"
#include "stringsep.h"
#include <cstddef>
#include <stdexcept>
#include <string_view>

inline bool is_escaped(std::string_view str, size_t pos) {
    return pos > 0 && str[pos - 1] == sep::ESCAPE_CHAR;
}

inline bool is_coment(std::string_view str, size_t pos) {
    return str[pos] == sep::COMMENT_CHAR && !is_escaped(str, pos);
}

inline bool is_com_sep(std::string_view str, size_t pos) {
    return str[pos] == sep::COMMAND_CHAR && !is_escaped(str, pos);
}

inline bool is_list_sep(std::string_view str, size_t pos) {
    const bool is_sep {str[pos] == sep::OR_CHAR || str[pos] == sep::AND_CHAR};
    return is_sep && !is_escaped(str, pos);
}

inline bool is_delimiter(std::string_view str, size_t pos, char delimeter) {
    return pos < str.size() && str[pos] == delimeter && !is_escaped(str, pos);
}

inline bool is_word_char(std::string_view str, size_t pos, char delimeter) {
    const bool escaped {is_escaped(str, pos)};
    return pos < str.size() && (str[pos] != delimeter || escaped)
    && (str[pos] != sep::COMMENT_CHAR || escaped);
}

args_container sh_tokenize(std::string_view line, char delimeter) {
    args_container res {};
    if (line.empty()) {
        return res;
    }

    size_t pos_start {};
    size_t pos_end {};
    do {
        if (line[pos_start] == sep::COMMENT_CHAR && !is_escaped(line, pos_start))
            break;

        bool in_squotes {false};
        bool in_dquotes {false};

        while (is_delimiter(line, pos_start, delimeter)) {
            pos_start++;
        }
        if (pos_start == line.size())
            break;

        pos_end = pos_start;
        while (is_word_char(line, pos_end, delimeter)) {
            if (line[pos_end] == '\'' && !in_dquotes && !is_escaped(line, pos_end)) {
                in_squotes = !in_squotes;
            } else if (line[pos_end] == '\"' && !in_squotes && !is_escaped(line, pos_end)) {
                in_dquotes = !in_dquotes;
            } else if (is_com_sep(line, pos_end) || is_list_sep(line, pos_end)) {
                if (pos_start != pos_end)
                    res.push_back(std::string(line.substr(pos_start, pos_end - pos_start)));
                pos_start = pos_end;
                pos_end++;
                if (pos_end < line.size() && is_list_sep(line, pos_end) && line[pos_end - 1] == line[pos_end]) {
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
