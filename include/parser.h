#pragma once

#include <string>
#include <string_view>
#include <vector>

std::vector<std::string> sh_tokenize(std::string_view line, char delimeter);
