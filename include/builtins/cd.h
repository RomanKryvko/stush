#pragma once

#include <string>
#include <filesystem>
#include <vector>

const int LOCATION_NOT_FOUND = 2;

int try_cd(const std::filesystem::path& path);

int com_cd(const std::vector<std::string>& args);
