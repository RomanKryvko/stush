#pragma once

#include <cstdint>
#include <string_view>

constexpr bool is_utf8(uint8_t c) {
    return c > 0x7F;
}

constexpr bool is_lead(uint8_t c) {
    //NOTE: 0xC0-0xC1 and 0xF5-0xFF are unused
    return c > 0xC1 && c < 0xF5;
}

constexpr int utf8_seq_length(uint8_t c) {
    if ((c & 0x80) == 0x00) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1;
}

constexpr size_t utf8_strlen(std::string_view s) {
    size_t res {};
    int i {0};
    while (i < s.size()) {
        i += utf8_seq_length(s.at(i));
        res++;
    }
    return res;
}
