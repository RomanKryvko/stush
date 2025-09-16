#include "cmd/expansion.h"
#include "cmd/cmd.h"
#include "cmd/variable.h"
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <glob.h>
#include <string>
#include <string_view>
#include <pwd.h>

static std::string word_separators {R"( \/$:;-+[]{}()'"?*)"};

[[nodiscard]]
std::string get_variable(const std::string& str) noexcept {
    const char* env {getenv(str.data())};
    if (!env) {
        if (var::is_set(str))
            return var::get_var(str);
        return "";
    }
    return env;
}

void expand_variable(std::string& str, size_t start_pos, size_t end_pos) {
    const std::string varname {str.substr(start_pos + 1, end_pos - start_pos - 1)};
    const auto expanded {get_variable(std::move(varname))};
    str.replace(start_pos, end_pos - start_pos, expanded);
}

void expand_word(std::string& str) {
    if (!str.empty() && str.front() == '\'' && str.back() == '\'')
        return;

    expand_tilde(str);

    bool escaped {false};
    size_t i {};
    while (i < str.size()) {
        const char c {str[i]};
        if (c == ESCAPE_CHAR && !escaped) {
            str.erase(i, 1);
            escaped = true;
            continue;
        }
        if (c == VAR_PREFIX && !escaped) {
            size_t varname_end {i + 1};
            while (varname_end < str.size() && word_separators.find(str[varname_end]) == std::string::npos) {
                varname_end++;
            }
            expand_variable(str, i, varname_end);
        }
        escaped = false;
        i++;
    }
}

[[nodiscard]]
std::string get_home(std::string_view name) {
    for (passwd* entry = getpwent(); entry != nullptr; entry = getpwent()) {
        if (entry->pw_name == name) {
            endpwent();
            return entry->pw_dir;
        }
    }
    endpwent();
    return "";
}

void expand_tilde(std::string& str) {
    if (str.empty() || str.at(0) != '~')
        return;

    const size_t slash_idx {str.find('/')};
    const size_t prefix_end {slash_idx == std::string::npos ? str.size() : slash_idx};

    if (prefix_end == 1) {
        const char* home {getenv("HOME")};
        if (home) {
            str.replace(0, 1, home);
        }
        return;
    }

    const std::string homedir {get_home(str.substr(1, prefix_end - 1))};
    if (homedir.size() == 1) {
        str.replace(0, prefix_end + 1, homedir);
    } else if (homedir.size() > 1) {
        str.replace(0, prefix_end, homedir);
    }
}

void expand_globs(args_container& args) {
    for (auto it = args.begin(); it != args.end(); it++) {
        if (it->find('*') == std::string::npos)
            continue;

        glob_t globbuf;
        glob(it->c_str(), GLOB_TILDE | GLOB_NOCHECK | GLOB_NOSORT, nullptr, &globbuf);
        if (globbuf.gl_pathc) {
            *it = globbuf.gl_pathv[0];
            for (int i = 1; i < globbuf.gl_pathc; i++) {
                it = args.insert(it, globbuf.gl_pathv[i]);
            }
        }
        globfree(&globbuf);
    }
}

constexpr bool is_quoted(std::string_view str) {
    return str.size() > 1 && str.front() == str.back() && (str.front() == '\'' || str.front() == '\"');
}

void strip_quotes(std::string& str) {
    if (is_quoted(str)) {
        str.erase(0, 1);
        str.erase(str.size() - 1, 1);
    }
}

void strip_all_quotes(args_view args) {
    for (auto& str : args) {
        strip_quotes(str);
    }
}
