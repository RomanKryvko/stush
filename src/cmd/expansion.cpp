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

void expand_variable(std::string& str, size_t start, size_t end) {
    const auto var_len {end - start};
    const auto expanded {get_variable(str.substr(start, var_len))};
    str.replace(start - 1, var_len + 1, expanded);
}

void expand_all_variables(std::string& str) {
    if (str.front() == '\'' && str.back() == '\'') // covers both str.size < 2 and quoted cases
        return;

    size_t var_idx {str.find(VAR_PREFIX)};

    while (var_idx != std::string::npos) {
        if (var_idx > 0 && str.at(var_idx - 1) == ESCAPE_CHAR) {
            str.erase(var_idx - 1, 1);
            var_idx = str.find(VAR_PREFIX, var_idx + 1);
            continue;
        }

        const size_t varname_start {var_idx + 1}; // Skip VAR_PREFIX
        size_t varname_end {varname_start};
        while (varname_end < str.size()) {
            if (word_separators.find(str.at(varname_end)) != std::string::npos) {
                expand_variable(str, varname_start, varname_end);
                break;
            }
            varname_end++;
        }
        if (varname_end == str.size()) {
            expand_variable(str, varname_start, varname_end);
            return;
        }

        var_idx = str.find(VAR_PREFIX, var_idx + 1);
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
