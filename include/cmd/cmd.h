#pragma once

#include <vector>
#include <string>

using args_container = std::vector<std::string>;

int run_simple_command(const args_container& args);

std::vector<args_container> split_pipelines(const args_container& args); //TODO: implement

std::vector<args_container> split_lists(const args_container& args);  //TODO: implement

std::vector<args_container> split_compound_command(const args_container& args);  //TODO: implement
