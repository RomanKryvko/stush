#include "cmd/cmd.h"
#include "linereader/linereader.h"
#include "parser.h"
#include <cassert>
#include <iostream>
#include <string>

int sh_main_loop(int argc, const char** argv) {
    char delimeter {' '};
    std::string prompt {">>> "};
    LineReader linereader {};
    while (true) {
        const std::string line {linereader.sh_read_line(prompt)};
        if (line.empty())
            continue;
        args_container args {sh_tokenize(line, delimeter)};
        std::cout << "\n";
        int status {run_compound_command(args)};
        std::cout << "\nProcess " << args[0] << " exited with code " << status << '\n';
    }
}

int main(int argc, char** argv) {
    sh_main_loop(argc, (const char**) argv);
}
