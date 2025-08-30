#include "linereader/linebuffer.h"

inline int LineBuffer::full_line_length() const {
    return buffer.size() + _line_start;
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

void LineBuffer::line_start(int col) {
    _line_start = col;
}

int LineBuffer::line_start() const {
    return _line_start;
}

const std::string& LineBuffer::get_text() const {
    return buffer;
}

void LineBuffer::set_text(const std::string& text) {
    buffer = text;
}

void LineBuffer::insert(key_code_t key) {
    if (cursor.col < full_line_length()) {
        buffer.insert(cursor_to_idx(), 1, key);
    } else {
        buffer += key;
    }
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

void LineBuffer::go_to_line_start() {
    cursor.col = _line_start;
}

void LineBuffer::go_to_line_end() {
    cursor.col = idx_to_cursor(buffer.size());
}

bool LineBuffer::erase_to_beginning() {
    if (buffer.empty())
        return false;

    buffer.erase(0, cursor_to_idx());
    go_to_line_start();

    return true;
}

bool LineBuffer::erase_to_end() {
    if (full_line_length() >= cursor.col) {
        buffer.erase(cursor_to_idx());
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
            buffer.erase(cursor_to_idx(), 1);
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

    const size_t adjusted_cursor {cursor_to_idx()};
    size_t space_idx {buffer.find_last_of(' ', adjusted_cursor + 1)};
    if (space_idx == std::string::npos) { // the string has no whitespaces
        return erase_to_beginning();
    }

    if (space_idx == adjusted_cursor - 1) { // cursor is on whitespace
        size_t char_idx {buffer.find_last_not_of(' ', adjusted_cursor)};
        if (char_idx == std::string::npos) { // the string consists of whitespaces
            return erase_to_beginning();
        }

        space_idx = buffer.find_last_of(' ', char_idx);
        if (space_idx == std::string::npos) {
            return erase_to_beginning();
        }
        buffer.erase(space_idx, char_idx - adjusted_cursor);
        cursor.col = idx_to_cursor(space_idx);
        return true;
    }
    buffer.erase(space_idx, adjusted_cursor - space_idx);
    cursor.col = idx_to_cursor(space_idx);
    return true;
}

bool LineBuffer::erase_word_forward() {
    if (buffer.empty() || cursor.col >= full_line_length())
        return false;

    const size_t adjusted_cursor {cursor_to_idx()};
    size_t space_idx {buffer.find_first_of(' ', adjusted_cursor)};
    if (space_idx == std::string::npos) { // the string has no whitespaces
        return erase_to_end();
    }

    if (space_idx == adjusted_cursor) { // cursor is on whitespace
        size_t char_idx {buffer.find_first_not_of(' ', adjusted_cursor)};
        if (char_idx == std::string::npos) { // the string consists of whitespaces
            return erase_to_end();
        }

        space_idx = buffer.find_first_of(' ', char_idx);
        if (space_idx == std::string::npos) {
            return erase_to_end();
        }
        buffer.erase(adjusted_cursor, space_idx - adjusted_cursor);
        return true;
    }
    buffer.erase(adjusted_cursor, space_idx - adjusted_cursor);
    return true;
}
