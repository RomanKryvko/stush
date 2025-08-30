#include "linereader/linebuffer.h"

int LineBuffer::full_line_length() {
    return buffer.size() + _line_start;
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
        buffer.insert(cursor.col - _line_start, 1, key);
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
    cursor.col = buffer.size() + _line_start;
}

bool LineBuffer::erase_to_beginning() {
    if (buffer.empty())
        return false;

    buffer.erase(0, cursor.col - _line_start);
    go_to_line_start();

    return true;
}

bool LineBuffer::erase_to_end() {
    if (full_line_length() >= cursor.col) {
        buffer.erase(cursor.col - _line_start);
        return true;
    }
    return false;
}

bool LineBuffer::erase_forward() {
    if (!buffer.empty() &&
        cursor.col > _line_start &&
        cursor.col < full_line_length())
    {
        buffer.erase(cursor.col - _line_start, 1);
        return true;
    }
    return false;
}

bool LineBuffer::erase_backwards() {
    if (!buffer.empty() && cursor.col > _line_start) {
        if (cursor.col < full_line_length()) {
            buffer.erase(cursor.col - _line_start, 1);
        } else {
            buffer.pop_back();
        }
        return true;
    }
    return false;
}
