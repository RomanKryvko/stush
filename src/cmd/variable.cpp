#include "cmd/variable.h"
#include <string>
#include <unordered_map>

std::unordered_map<std::string, std::string> shell_vars {};

void var::set_var(const std::string& var, const std::string& value) noexcept {
    shell_vars[var] = value;
}

bool var::is_set(const std::string& var) noexcept {
    return shell_vars.contains(var);
}

void var::unset(const std::string& var) noexcept {
    if (shell_vars.contains(var))
        shell_vars.erase(var);
}

const std::string& var::get_var(const std::string& var) {
    return shell_vars.at(var);
}
