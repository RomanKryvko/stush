#pragma once

#include "linereader/utf8utils.h"
#include <cstddef>
#include <iterator>
#include <string>
#include <string_view>

class utf8string {
    std::string buffer {};
    size_t _char_size {};

    std::string get_out_of_range_msg(int idx) const;
    utf8string& append_utf8(const utf8string& other);
    utf8string& append_char32(char32_t c);

public:
    template <typename PtrType>
    class basic_iterator {
        PtrType buf;
        size_t byte_index;

    public:
        using value_type = char32_t;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        basic_iterator() = default;
        basic_iterator(PtrType b, size_t idx) : buf(b), byte_index(idx) {}

        bool operator==(const basic_iterator& other) const {
            return buf == other.buf && byte_index == other.byte_index;
        }
        bool operator!=(const basic_iterator& other) const {
            return !(*this == other);
        }

        char32_t operator*() const {
            if (!buf || byte_index >= buf->size()) return -1;
            return utf8utils::char_at(*buf, byte_index);
        }

        // Prefix
        basic_iterator& operator++() {
            if (buf && byte_index < buf->size()) {
                const unsigned char c {static_cast<unsigned char>((*buf)[byte_index])};
                byte_index += utf8utils::utf8_seq_length(c);
            }
            return *this;
        }

        // Postfix
        basic_iterator operator++(int) {
            basic_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        // Prefix
        basic_iterator& operator--() {
            while (buf && byte_index > 0) {
                byte_index--;
                const unsigned char c {static_cast<unsigned char>((*buf)[byte_index])};
                if (utf8utils::is_lead(c) || isascii(c))
                    break;
            }
            return *this;
        }

        // Postfix
        basic_iterator operator--(int) {
            basic_iterator tmp = *this;
            --(*this);
            return tmp;
        }
    };

    using const_iterator = basic_iterator<const std::string*>;
    using iterator = basic_iterator<std::string*>;

    static_assert(std::forward_iterator<const_iterator>);
    static_assert(std::bidirectional_iterator<const_iterator>);

    iterator begin();
    iterator end();

    const_iterator cbegin() const;
    const_iterator cend() const;

    iterator it_at(size_t pos);
    const_iterator cit_at(size_t pos) const;

    utf8string();
    utf8string(const std::string& str);
    utf8string(const char* str);
    utf8string(std::string_view str);

    bool operator ==(const utf8string& other) const;
    bool operator !=(const utf8string& other) const;

    utf8string& operator +=(std::string_view str);
    utf8string& operator +(std::string_view str);
    utf8string& operator +=(const utf8string& other);
    utf8string& operator +(const utf8string& other);
    utf8string& operator +=(char32_t c);
    utf8string& operator +(char32_t c);
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
