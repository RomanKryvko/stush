#pragma once

#include <string>

namespace var {

void set_var(const std::string& var, const std::string& value) noexcept;

bool is_set(const std::string& var) noexcept;

void unset(const std::string& var) noexcept;

const std::string& get_var(const std::string& var);

}
