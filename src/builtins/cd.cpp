#include "builtins/builtins.h"
#include "cmd/cmd.h"
#include "builtins/cd.h"
#include <iostream>
#include <unistd.h>
#include <wait.h>

namespace fs = std::filesystem;

int try_cd(const fs::path& path) {
    try {
        fs::current_path(fs::canonical(path));
        return EXIT_SUCCESS;
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << '\n';
        return LOCATION_NOT_FOUND;
    }
}

int com_cd(args_view args) {
    if (args.size() == 1) {
        char* home = getenv("HOME");
        if (home && fs::is_directory(home)) {
            return try_cd(home);
        } else {
            std::cerr << "Couldn't locate HOME\n";
            return LOCATION_NOT_FOUND;
        }
    }
    if (args.size() > 2){
        err_too_many_args(args[0]);
        return EXIT_FAILURE;
    }
    return try_cd(args[1]);
}

