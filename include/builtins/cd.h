#pragma once

#include "cmd/cmd.h"
#include <filesystem>

const int LOCATION_NOT_FOUND = 2;

int try_cd(const std::filesystem::path& path);

int com_cd(const args_container& args);
