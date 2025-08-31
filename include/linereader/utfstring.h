#pragma once

#include <cstddef>
#include <string>
#include <string_view>

class utf8string {
    std::string buffer {};
    size_t _char_size {};

    std::string get_out_of_range_msg(int idx) const;

public:
    utf8string(const std::string& str);
    utf8string(const char* str);
    utf8string(std::string_view str);

    size_t byte_size() const;
    size_t char_size() const;

    const std::string& stdstr() const;
    void stdstr(const std::string& str);

    size_t char_to_byte(size_t char_idx) const;

    std::string at(size_t char_idx) const;

    void insert(size_t char_idx, std::string_view utf8_char);
    void insert(size_t char_idx, char chr);

    // Erase single UTF-8 character at idx
    void erase_at(size_t pos);
    void erase(size_t pos = 0, size_t n = std::string::npos);
};
