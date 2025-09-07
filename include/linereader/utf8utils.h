#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace utf8utils {

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

constexpr char32_t char_at(std::string_view str, size_t byte_idx) {
    const unsigned char c {(unsigned char)(str.at(byte_idx))};
    const int len {utf8_seq_length(c)};
    if (len == 1) {
        return c;
    }

    char32_t codepoint {};
    switch (len) {
        case 2: {
            codepoint = (c & 0x1F) << 6;
            codepoint |= (str[byte_idx+1] & 0x3F);
            break;
        }
        case 3: {
            codepoint = (c & 0x0F) << 12;
            codepoint |= (str[byte_idx+1] & 0x3F) << 6;
            codepoint |= str[byte_idx+2] & 0x3F;
            break;
        }
        case 4: {
            codepoint = (c & 0x07) << 18;
            codepoint |= (str[byte_idx+1] & 0x3F) << 12;
            codepoint |= (str[byte_idx+2] & 0x3F) << 6;
            codepoint |= str[byte_idx+3] & 0x3F;
            break;
        }
        //TODO: handle invalid characters
    }

    return codepoint;
}

constexpr int append(std::string& str, char32_t c) {
    if (c <= 0x7F) {
        str += c;
        return 1;
    } else if (c <= 0x7FF) {
        str += 0xC0 | ((c >> 6) & 0x1F);
        str += 0x80 | (c & 0x3F);
        return 2;
    } else if (c <= 0xFFFF) {
        str += 0xE0 | ((c >> 12) & 0x0F);
        str += 0x80 | ((c >> 6) & 0x3F);
        str += 0x80 | (c & 0x3F);
        return 3;
    } else if (c <= 0x10FFFF) {
        str += 0xF0 | ((c >> 18) & 0x07);
        str += 0x80 | ((c >> 12) & 0x3F);
        str += 0x80 | ((c >> 6) & 0x3F);
        str += 0x80 | (c & 0x3F);
        return 4;
    }
    return 1; //TODO: handle invalid characters
}

}
