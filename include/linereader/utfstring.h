#pragma once

#include <cstddef>
#include <string>
#include <string_view>

class utf8string {
    std::string buffer {};
    size_t _char_size {};

    std::string get_out_of_range_msg(int idx) const;
    utf8string& append_utf8(const utf8string& other);

public:
    utf8string();
    utf8string(const std::string& str);
    utf8string(const char* str);
    utf8string(std::string_view str);

    utf8string& operator +=(std::string_view str);
    utf8string& operator +(std::string_view str);
    utf8string& operator +=(const utf8string& other);
    utf8string& operator +(const utf8string& other);
    utf8string& append(std::string_view str);

    size_t byte_size() const;
    size_t char_size() const;
    bool empty() const;

    const std::string& stdstr() const;
    void stdstr(const std::string& str);

    size_t char_to_byte(size_t char_idx) const;
    size_t byte_to_char(size_t byte_idx) const;

    char32_t at(size_t char_idx) const;

    void insert(size_t char_idx, std::string_view utf8_char);
    void insert(size_t char_idx, char chr);
    void insert_utf8(size_t char_idx, const utf8string& other);

    // Erase single UTF-8 character at idx
    void erase_at(size_t pos);
    void erase(size_t pos = 0, size_t n = std::string::npos);
    void pop_back();

    [[nodiscard]]
    utf8string substr(size_t pos = 0, size_t n = std::string::npos) const;

    [[nodiscard]]
    size_t find_last_of(char c, size_t pos = std::string::npos) const;
    [[nodiscard]]
    size_t find_last_not_of(char c, size_t pos = std::string::npos) const;

    [[nodiscard]]
    size_t find_last_of(std::string_view s, size_t pos = std::string::npos) const;
    [[nodiscard]]
    size_t find_last_not_of(std::string_view s, size_t pos = std::string::npos) const;

    [[nodiscard]]
    size_t find_first_of(char c, size_t pos = 0) const;
    [[nodiscard]]
    size_t find_first_not_of(char c, size_t pos = 0) const;

    [[nodiscard]]
    size_t find_first_of(std::string_view s, size_t pos = 0) const;
    [[nodiscard]]
    size_t find_first_not_of(std::string_view s, size_t pos = 0) const;
};
