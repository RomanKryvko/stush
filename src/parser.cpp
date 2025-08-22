#include <parser.h>

std::vector<std::string> sh_tokenize(std::string_view line, char delimeter) {
    std::vector<std::string> res {};
    if (line.empty()) {
        return res;
    }

    size_t pos_start {};
    size_t pos_end {};
    do {
        pos_start = line.find_first_not_of(delimeter, pos_start);
        if (pos_start == std::string_view::npos) {
            break;
        }
        std::string token {};
        pos_end = line.find(delimeter, pos_start);
        if (pos_end != std::string_view::npos) {
            token = line.substr(pos_start, pos_end - pos_start);
        } else {
            token = line.substr(pos_start);
        }
        res.push_back(token);
        pos_start = pos_end + 1;
    } while (pos_end != std::string_view::npos);

    return res;
}
