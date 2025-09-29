#include "cmd/cmd.h"
#include "linereader/linereader.h"
#include "parser.h"
#include "stringsep.h"
#include <bits/getopt_core.h>
#include <cassert>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <sys/types.h>

namespace fs = std::filesystem;

const std::string_view DELIMETER {" \t"};

int sh_main_loop(int argc, const char** argv) {
    std::string prompt {">>> "};
    LineReader linereader {};
    while (true) {
        const std::string line {linereader.sh_read_line(prompt)};
        if (line.empty())
            continue;
        args_container args {sh_tokenize(line, DELIMETER)};
        std::cout << "\n";
        int status {run_compound_command(args)};
        std::cout << "\nProcess " << args[0] << " exited with code " << status << '\n';
    }
}

[[nodiscard]]
int run_command(std::string_view command) {
    args_container args {sh_tokenize(command, DELIMETER)};
    return run_compound_command(args);
}

int main(int argc, char** argv) {
    int opt {};
    while ((opt = getopt(argc, argv, "c:")) != -1) {
        switch (opt) {
            case 'c': {
                const int status {run_command(optarg)};
                exit(status);
                break;
            }
            default: {
                std::cerr << "Usage: stush [option] script-file\n"
                    "Options:\n\t-c\texecute a command.\n";
                exit(EXIT_FAILURE);
            }
        }
    }
    if (optind < argc) {
        try {
            const auto filename {fs::canonical(argv[optind])};
            if (!fs::is_regular_file(filename) && !fs::is_symlink(filename)) {
                std::cerr << "Cannot execute " << std::quoted(argv[optind]) <<
                    ": is not a regular file or a symlink.\n";
                exit(EXIT_FAILURE);
            }

            std::ifstream ifs {filename};
            std::string line {};
            int status {};
            while (std::getline(ifs, line)) {
                if (!line.empty() && line.front() != sep::COMMENT_CHAR)
                    status = run_command(line);
            }
            exit(status);
        } catch (const fs::filesystem_error& err) {
            std::cerr << err.what() << '\n';
            exit(EXIT_FAILURE);
        }
    }

    signal(SIGINT, SIG_IGN);
    sh_main_loop(argc, (const char**) argv);
}
