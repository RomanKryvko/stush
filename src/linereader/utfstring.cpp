#include "linereader/utf8utils.h"
#include "linereader/utfstring.h"
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

std::string utf8string::get_out_of_range_msg(int idx) const {
    return "Idx " + std::to_string(idx) +
    " can not be greater then byte size (" +
    std::to_string(buffer.size()) + ")";
}

utf8string::utf8string() = default;

utf8string::utf8string(const std::string& str) :
    buffer(str),
    _char_size(utf8_strlen(str))
{ }

utf8string::utf8string(const char* str) :
    buffer(str),
    _char_size(utf8_strlen(str))
{ }

utf8string::utf8string(std::string_view str) :
    buffer(str),
    _char_size(utf8_strlen(str))
{ }

size_t utf8string::byte_size() const {
    return buffer.size();
}

size_t utf8string::char_size() const {
    return _char_size;
}

bool utf8string::empty() const {
    return buffer.empty();
}

const std::string& utf8string::stdstr() const {
    return buffer;
}

void utf8string::stdstr(const std::string& str) {
    buffer = str;
    _char_size = utf8_strlen(buffer);
}

size_t utf8string::char_to_byte(size_t char_idx) const {
    if (char_idx == std::string::npos)
        return char_idx;

    size_t i {};
    size_t chars {};
    while (i < buffer.size() && chars < char_idx) {
        i += utf8_seq_length((uint8_t)buffer.at(i));
        chars++;
    }
    return i;
}

size_t utf8string::byte_to_char(size_t byte_idx) const {
    if (byte_idx == std::string::npos)
        return byte_idx;

    size_t bytes {};
    size_t chars {};
    while (bytes < byte_idx) {
        bytes += utf8_seq_length((uint8_t)buffer.at(bytes));
        chars++;
    }
    return chars;
}

std::string utf8string::at(size_t char_idx) const {
    size_t byte_idx {char_to_byte(char_idx)};
    if (byte_idx >= buffer.size())
        throw std::out_of_range(get_out_of_range_msg(byte_idx));

    int len {utf8_seq_length((uint8_t)buffer.at(byte_idx))};
    return buffer.substr(byte_idx, len);
}

bool utf8string::equals_at(size_t char_idx, std::string_view s) const {
    const size_t byte_idx {char_to_byte(char_idx)};
    for (int i = 0; i < s.size(); i++) {
        if (s.at(i) != buffer.at(byte_idx + i))
            return false;
    }
    return true;
}

void utf8string::insert(size_t char_idx, std::string_view utf8_char) {
    size_t byte_idx {char_to_byte(char_idx)};
    _char_size += utf8_strlen(utf8_char);
    buffer.insert(byte_idx, utf8_char);
}

void utf8string::insert(size_t char_idx, char chr) {
    size_t byte_idx {char_to_byte(char_idx)};
    _char_size++;
    buffer.insert(byte_idx, 1, chr);
}

void utf8string::insert_utf8(size_t char_idx, const utf8string& other) {
    size_t byte_idx {char_to_byte(char_idx)};
    _char_size += other.char_size();
    buffer.insert(byte_idx, other.stdstr());
}

// Erase single UTF-8 character at idx
void utf8string::erase_at(size_t pos) {
    size_t byte_idx {char_to_byte(pos)};
    if (byte_idx >= buffer.size())
        throw std::out_of_range(get_out_of_range_msg(byte_idx));

    int len {utf8_seq_length((uint8_t)buffer.at(byte_idx))};
    _char_size--;
    buffer.erase(byte_idx, len);
}

void utf8string::erase(size_t pos, size_t n) {
    // Mimics behavior of std::string::erase
    if (buffer.empty() && pos == 0 && n == std::string::npos)
        return;

    size_t start_idx {char_to_byte(pos)};
    size_t end_idx {char_to_byte(pos + n)};
    if (start_idx >= buffer.size())
        throw std::out_of_range(get_out_of_range_msg(start_idx));

    if (n == std::string::npos || pos + n > _char_size) {
        buffer.erase(start_idx);
        _char_size = pos;
        return;
    }

    auto diff {end_idx - start_idx};
    buffer.erase(start_idx, diff);
    _char_size -= n;
}

utf8string& utf8string::append(std::string_view str) {
    _char_size += utf8_strlen(str);
    buffer += str;
    return *this;
}

utf8string& utf8string::operator +=(std::string_view str) {
    return append(str);
}

utf8string& utf8string::operator +(std::string_view str) {
    return append(str);
}

utf8string& utf8string::append_utf8(const utf8string& other) {
    buffer += other.stdstr();
    _char_size += other.char_size();
    return *this;
}

utf8string& utf8string::operator +=(const utf8string& other) {
    return append_utf8(other);
}

utf8string& utf8string::operator +(const utf8string& other) {
    return append_utf8(other);
}

void utf8string::pop_back() {
    if (buffer.empty())
        throw std::out_of_range("String must be not empty to pop.");

    erase_at(_char_size - 1);
}

utf8string utf8string::substr(size_t pos, size_t n) const {
    // Mimics behavior of std::string::erase
    if (buffer.empty() && pos == 0 && n == std::string::npos)
        return *this;

    size_t start_idx {char_to_byte(pos)};
    size_t end_idx {char_to_byte(pos + n)};
    if (start_idx >= buffer.size())
        throw std::out_of_range(get_out_of_range_msg(start_idx));

    if (n == std::string::npos || pos + n > _char_size)
        return buffer.substr(start_idx);

    auto diff {end_idx - start_idx};
    return buffer.substr(start_idx, diff);
}

size_t utf8string::find_last_of(char c, size_t pos) const {
    const size_t byte_idx = char_to_byte(pos);

    const size_t res {buffer.find_last_of(c, byte_idx)};

    return byte_to_char(res); //TODO:  find a better performing way to do it
}

size_t utf8string::find_last_not_of(char c, size_t pos) const {
    const size_t byte_idx = char_to_byte(pos);

    const size_t res {buffer.find_last_not_of(c, byte_idx)};

    return byte_to_char(res);
}

size_t utf8string::find_last_of(std::string_view s, size_t pos) const {
    const size_t byte_idx = char_to_byte(pos);

    const size_t res {buffer.find_last_of(s, byte_idx)};

    return byte_to_char(res);
}

size_t utf8string::find_last_not_of(std::string_view s, size_t pos) const {
    const size_t byte_idx = char_to_byte(pos);

    const size_t res {buffer.find_last_not_of(s, byte_idx)};

    return byte_to_char(res);
}

size_t utf8string::find_first_of(char c, size_t pos) const {
    const size_t byte_idx = char_to_byte(pos);

    const size_t res {buffer.find_first_of(c, byte_idx)};

    return byte_to_char(res);
}

size_t utf8string::find_first_not_of(char c, size_t pos) const {
    const size_t byte_idx = char_to_byte(pos);

    const size_t res {buffer.find_first_not_of(c, byte_idx)};

    return byte_to_char(res);
}

size_t utf8string::find_first_of(std::string_view s, size_t pos) const {
    const size_t byte_idx = char_to_byte(pos);

    const size_t res {buffer.find_first_of(s, byte_idx)};

    return byte_to_char(res);
}

size_t utf8string::find_first_not_of(std::string_view s, size_t pos) const {
    const size_t byte_idx = char_to_byte(pos);

    const size_t res {buffer.find_first_not_of(s, byte_idx)};

    return byte_to_char(res);
}
