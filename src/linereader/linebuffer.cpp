#include "linereader/linebuffer.h"
#include "byteutils.h"
#include <cstddef>
#include <cstdlib>
#include <string>

inline int LineBuffer::full_line_length() const {
    return buffer.char_size() + _line_start;
}

inline size_t LineBuffer::idx_to_cursor() const {
    return cursor.col + _line_start;
}

inline size_t LineBuffer::idx_to_cursor(int col) const {
    return col + _line_start;
}

inline size_t LineBuffer::cursor_to_idx() const {
    return cursor.col - _line_start;
}

inline void LineBuffer::cut(size_t pos, size_t n) {
    yank_buffer = buffer.substr(pos, n);
    buffer.erase(pos, n);
}

void LineBuffer::line_start(int col) {
    _line_start = col;
}

int LineBuffer::line_start() const {
    return _line_start;
}

void LineBuffer::word_separators(const std::string& separators) {
    _word_separators = separators;
}
const std::string& LineBuffer::word_separators() const {
    return _word_separators;
}

const std::string& LineBuffer::get_text() const {
    return buffer.stdstr();
}

void LineBuffer::set_text(const std::string& text) {
    buffer = text;
}

bool LineBuffer::insert(key_code_t key) {
    const auto bytes {highest_nonzero_byte(key)}; //NOTE: number of bytes is 0-indexed
    if (bytes == -1)
        return false;

    const auto insert_at = [&](std::string_view s) {
        size_t old_len {buffer.char_size()};
        if (cursor.col < full_line_length())
            buffer.insert(cursor_to_idx(), s);
        else
            buffer += s;

        cursor.col += buffer.char_size() - old_len;
    };

    if (bytes == 0) {
        const char c {static_cast<char>(key)};
        insert_at(std::string_view(&c, 1));
    } else {
        const auto str {unpack_str(key)};
        insert_at(str);
    }
    return true;
}

cursor_pos LineBuffer::cursor_position() const {
    return cursor;
}

void LineBuffer::cursor_position(cursor_pos position) {
    cursor = position;
}

bool LineBuffer::move_cursor_right() {
    if (cursor.col < full_line_length()) {
        cursor.col++;
        return true;
    }
    return false;
}

bool LineBuffer::move_cursor_left() {
    if (cursor.col > _line_start) {
        cursor.col--;
        return true;
    }
    return false;
}

void LineBuffer::jump_word_right() {
    if (buffer.empty() || cursor.col >= full_line_length())
        return;

    const size_t adjusted_cursor {cursor_to_idx()};
    size_t idx {std::string::npos};

    char selected_char {};
    for (const auto c : _word_separators) {
        size_t cur {adjusted_cursor + 1};
        while (cur < buffer.char_size() && !buffer.equals_at(cur, c)) {
            cur++;
        }

        if (cur < idx) {
            idx = cur;
            selected_char = c;
        }
    }

    if (idx == buffer.char_size()) {
        cursor.col = full_line_length();
    } else {
        // Skip same separators
        while (idx < buffer.char_size() && buffer.equals_at(idx + 1, selected_char)) {
            idx++;
        }
        cursor.col = idx_to_cursor(idx);
    }
}

void LineBuffer::jump_word_left() {
    if (buffer.empty() || cursor.col <= _line_start)
        return;

    const size_t adjusted_cursor {cursor_to_idx()};
    size_t idx {0};

    char selected_char {};
    for (const auto c : _word_separators) {
        size_t cur {adjusted_cursor - 1};
        while (cur > 0 && !buffer.equals_at(cur, c)) {
            cur--;
        }

        if (cur > idx) {
            idx = cur;
            selected_char = c;
        }
    }

    if (idx <= 0) {
        cursor.col = _line_start;
    } else {
        // Skip same separators
        while (idx > 0 && buffer.equals_at(idx - 1, selected_char)) {
            idx--;
        }
        cursor.col = idx_to_cursor(idx);
    }
}

void LineBuffer::go_to_line_start() {
    cursor.col = _line_start;
}

void LineBuffer::go_to_line_end() {
    cursor.col = idx_to_cursor(buffer.char_size());
}

bool LineBuffer::erase_to_beginning() {
    if (buffer.empty())
        return false;

    cut(0, cursor_to_idx());
    go_to_line_start();

    return true;
}

bool LineBuffer::erase_to_end() {
    if (full_line_length() >= cursor.col) {
        cut(cursor_to_idx(), buffer.char_size() - cursor_to_idx());
        return true;
    }
    return false;
}

bool LineBuffer::erase_forward() {
    if (!buffer.empty() &&
        cursor.col >= _line_start &&
        cursor.col < full_line_length())
    {
        buffer.erase(cursor_to_idx(), 1);
        return true;
    }
    return false;
}

bool LineBuffer::erase_backwards() {
    if (!buffer.empty() && cursor.col > _line_start) {
        if (cursor.col < full_line_length()) {
            buffer.erase(cursor_to_idx() - 1, 1);
        } else {
            buffer.pop_back();
        }
        return true;
    }
    return false;
}

bool LineBuffer::erase_word_backwards() {
    if (buffer.empty() || cursor.col <= _line_start)
        return false;

    const size_t init_cur {cursor_to_idx()};
    size_t cur {init_cur};

    while (cur > 0 && buffer.equals_at(cur - 1, " ")) {
        cur--;
    }

    if (cur == 0)
        return erase_to_beginning();

    while (cur > 0 && !buffer.equals_at(cur - 1, " ")) {
        cur--;
    }

    cut(cur, init_cur - cur);
    cursor.col = idx_to_cursor(cur);

    return true;
}

bool LineBuffer::erase_word_forward() {
    if (buffer.empty() || cursor.col >= full_line_length())
        return false;

    const size_t init_cur {cursor_to_idx()};
    size_t cur {init_cur};

    while (cur < buffer.char_size() && buffer.equals_at(cur, " ")) {
        cur++;
    }

    if (cur == buffer.char_size())
        return erase_to_end();

    while (cur < buffer.char_size() && !buffer.equals_at(cur, " ")) {
        cur++;
    }

    cut(init_cur, cur - init_cur);

    return true;
}

bool LineBuffer::paste() {
    if (yank_buffer.empty())
        return false;

    if (cursor.col < full_line_length()) {
        buffer.insert_utf8(cursor_to_idx(), yank_buffer);
    } else {
        buffer += yank_buffer;
    }
    cursor.col += yank_buffer.char_size();
    return true;
}
