#include <gtest/gtest.h>
#include "linereader/linebuffer.h"
#include "linereader/types.h"

TEST(LineBufferTest, cursorDoesntOvershootLine) {
    LineBuffer linebuffer {}; // buffer is empty initially

    cursor_pos start {1, 1};
    linebuffer.cursor_position(start);
    const bool status {linebuffer.move_cursor_right()};

    EXPECT_FALSE(status);
    EXPECT_EQ(linebuffer.cursor_position(), start);
}

TEST(LineBufferTest, cursorDoesntGoBeforeLineStart) {
    LineBuffer linebuffer {}; // buffer is empty initially

    cursor_pos start {1, 5};
    linebuffer.line_start(5);
    linebuffer.cursor_position(start);
    const bool status {linebuffer.move_cursor_left()};

    EXPECT_FALSE(status);
    EXPECT_EQ(linebuffer.cursor_position(), start);
}

TEST(LineBufferTest, goesToLineStart) {
    LineBuffer linebuffer {}; // buffer is empty initially
    cursor_pos start {1, 10};

    linebuffer.set_text("test");
    linebuffer.line_start(2);
    linebuffer.cursor_position(start);
    linebuffer.go_to_line_start();

    cursor_pos exp {1, 2};
    EXPECT_EQ(linebuffer.cursor_position(), exp);
}

TEST(LineBufferTest, goesToLineEnd) {
    LineBuffer linebuffer {};

    const std::string test_text {"test"};
    const int line_start {2};
    const cursor_pos start {1, 1};

    linebuffer.set_text(test_text);
    linebuffer.line_start(line_start);
    linebuffer.cursor_position(start);
    linebuffer.go_to_line_end();

    cursor_pos exp {1, (int) test_text.size() + line_start};
    EXPECT_EQ(linebuffer.cursor_position(), exp);
}

TEST(LineBufferTest, erasesToBeginning) {
    LineBuffer linebuffer {};

    const std::string test_text {"test"};
    const int line_start {2};
    const cursor_pos start {1, (int) test_text.size() + line_start};
    const cursor_pos exp {1, line_start};

    linebuffer.line_start(line_start);
    linebuffer.set_text(test_text);
    linebuffer.cursor_position(start);
    const bool status {linebuffer.erase_to_beginning()};

    EXPECT_TRUE(status);
    EXPECT_EQ(linebuffer.get_text(), "");
    EXPECT_EQ(linebuffer.cursor_position(), exp);
}

TEST(LineBufferTest, insertAtEnd) {
    LineBuffer linebuffer {};
    cursor_pos start {1, 4};

    linebuffer.set_text("foo");
    linebuffer.cursor_position(start); // after last char
    linebuffer.insert('x'); // insert does not move the cursor by itself

    EXPECT_EQ(linebuffer.get_text(), "foox");
    EXPECT_EQ(linebuffer.cursor_position(), start);
}

TEST(LineBufferTest, insertAtBeginning) {
    LineBuffer linebuffer {};
    cursor_pos start {1, 1};

    linebuffer.line_start(1);
    linebuffer.set_text("bar");
    linebuffer.cursor_position(start);
    linebuffer.insert('X');

    EXPECT_EQ(linebuffer.get_text(), "Xbar");
    EXPECT_EQ(linebuffer.cursor_position(), start);
}

TEST(LineBufferTest, insertInMiddle) {
    LineBuffer linebuffer {};
    cursor_pos start {1, 3};

    linebuffer.set_text("abcd");
    linebuffer.line_start(1);
    linebuffer.cursor_position(start); // between b and c
    linebuffer.insert('X');

    EXPECT_EQ(linebuffer.get_text(), "abXcd");
    EXPECT_EQ(linebuffer.cursor_position(), start);
}

TEST(LineBufferTest, eraseBackwardAtEnd) {
    LineBuffer linebuffer {};
    cursor_pos start {1, 4};

    linebuffer.set_text("test");
    linebuffer.line_start(1);
    linebuffer.cursor_position(start); // after 't'

    bool status = linebuffer.erase_backwards();
    EXPECT_TRUE(status);
    EXPECT_EQ(linebuffer.get_text(), "tes");
    EXPECT_EQ(linebuffer.cursor_position(), start);
}

TEST(LineBufferTest, eraseBackwardAtBeginningDoesNothing) {
    LineBuffer linebuffer {};
    cursor_pos start {1, 1};

    linebuffer.set_text("test");
    linebuffer.line_start(1);
    linebuffer.cursor_position(start);

    bool status = linebuffer.erase_backwards();
    EXPECT_FALSE(status);
    EXPECT_EQ(linebuffer.get_text(), "test");
    EXPECT_EQ(linebuffer.cursor_position(), start);
}

TEST(LineBufferTest, eraseForwardInMiddle) {
    LineBuffer linebuffer {};
    cursor_pos start {1, 3};

    linebuffer.set_text("hello");
    linebuffer.line_start(1);
    linebuffer.cursor_position(start); // on 'l'

    bool status = linebuffer.erase_forward();
    EXPECT_TRUE(status);
    EXPECT_EQ(linebuffer.get_text(), "helo");
    EXPECT_EQ(linebuffer.cursor_position(), start);
}

TEST(LineBufferTest, eraseForwardAtEndDoesNothing) {
    LineBuffer linebuffer {};
    cursor_pos start {1, 3};

    linebuffer.set_text("hi");
    linebuffer.line_start(1);
    linebuffer.cursor_position(start); // after last char

    bool status = linebuffer.erase_forward();
    EXPECT_FALSE(status);
    EXPECT_EQ(linebuffer.get_text(), "hi");
    EXPECT_EQ(linebuffer.cursor_position(), start);
}

TEST(LineBufferTest, erasesWordBackwards) {
    LineBuffer linebuffer {};
    cursor_pos start {1, 8};

    linebuffer.line_start(1);
    linebuffer.set_text("foo bar");
    linebuffer.cursor_position(start);

    bool status = linebuffer.erase_word_backwards();
    EXPECT_TRUE(status);
    EXPECT_EQ(linebuffer.get_text(), "foo");
    EXPECT_EQ(linebuffer.cursor_position(), cursor_pos(1, 4));
}

TEST(LineBufferTest, erasesWordBackwardsWithWhitespaces) {
    LineBuffer linebuffer {};
    cursor_pos start {1, 14};

    linebuffer.line_start(1);
    linebuffer.set_text("foo bar      "); // cursor is at the end
    linebuffer.cursor_position(start);

    bool status = linebuffer.erase_word_backwards();
    EXPECT_TRUE(status);
    EXPECT_EQ(linebuffer.get_text(), "foo");
    EXPECT_EQ(linebuffer.cursor_position(), cursor_pos(1, 4));
}

TEST(LineBufferTest, erasesAllWhenNoWhitespacesPresent) {
    LineBuffer linebuffer {};
    cursor_pos start {1, 14};

    linebuffer.line_start(1);
    linebuffer.set_text("foobarfoobar"); // cursor is at the end
    linebuffer.cursor_position(start);

    bool status = linebuffer.erase_word_backwards();
    EXPECT_TRUE(status);
    EXPECT_TRUE(linebuffer.get_text().empty());
    EXPECT_EQ(linebuffer.cursor_position(), cursor_pos(1, 1));
}

TEST(LineBufferTest, erasesAllWhenBufferIsBlank) {
    LineBuffer linebuffer {};
    cursor_pos start {1, 13};

    linebuffer.line_start(1);
    linebuffer.set_text("            "); // cursor is at the end
    linebuffer.cursor_position(start);

    bool status = linebuffer.erase_word_backwards();
    EXPECT_TRUE(status);
    EXPECT_TRUE(linebuffer.get_text().empty());
    EXPECT_EQ(linebuffer.cursor_position(), cursor_pos(1, 1));
}

TEST(LineBufferTest, erasesFromWordMiddle) {
    LineBuffer linebuffer {};

    linebuffer.line_start(1);
    linebuffer.set_text("foo bar foo");
    cursor_pos start {1, 10};
    linebuffer.cursor_position(start);

    bool status = linebuffer.erase_word_backwards();
    EXPECT_TRUE(status);
    EXPECT_EQ(linebuffer.get_text(), "foo baroo");
    EXPECT_EQ(linebuffer.cursor_position(), cursor_pos(1, 8));
}

TEST(LineBufferTest, doesntEraseWordWhenBufferEmpty) {
    LineBuffer linebuffer {};
    cursor_pos start {1, 2};

    linebuffer.line_start(1);
    linebuffer.set_text("");
    linebuffer.cursor_position(start);

    bool status = linebuffer.erase_word_backwards();
    EXPECT_FALSE(status);
    EXPECT_EQ(linebuffer.cursor_position(), start);
}

TEST(LineBufferTest, erasesWordForward) {
    LineBuffer linebuffer {};
    cursor_pos start {1, 1};

    linebuffer.line_start(1);
    linebuffer.set_text("foo bar"); // cursor is on 'f'
    linebuffer.cursor_position(start);

    bool status = linebuffer.erase_word_forward();
    EXPECT_TRUE(status);
    EXPECT_EQ(linebuffer.get_text(), " bar");
    EXPECT_EQ(linebuffer.cursor_position(), start);
}

TEST(LineBufferTest, erasesWordForwardWithWhitespaces) {
    LineBuffer linebuffer {};
    cursor_pos start {1, 1};

    linebuffer.line_start(1);
    linebuffer.set_text("      foo bar");
    linebuffer.cursor_position(start);

    bool status = linebuffer.erase_word_forward();
    EXPECT_TRUE(status);
    EXPECT_EQ(linebuffer.get_text(), " bar");
    EXPECT_EQ(linebuffer.cursor_position(), start);
}

TEST(LineBufferTest, erasesAllForwardWhenNoWhitespacesPresent) {
    LineBuffer linebuffer {};
    cursor_pos start {1, 1};

    linebuffer.line_start(1);
    linebuffer.set_text("foobarfoobar");
    linebuffer.cursor_position(start);

    bool status = linebuffer.erase_word_forward();
    EXPECT_TRUE(status);
    EXPECT_TRUE(linebuffer.get_text().empty());
    EXPECT_EQ(linebuffer.cursor_position(), start);
}

TEST(LineBufferTest, erasesAllForwardWhenBufferIsBlank) {
    LineBuffer linebuffer {};
    cursor_pos start {1, 1};

    linebuffer.line_start(1);
    linebuffer.set_text("            ");
    linebuffer.cursor_position(start);

    bool status = linebuffer.erase_word_forward();
    EXPECT_TRUE(status);
    EXPECT_TRUE(linebuffer.get_text().empty());
    EXPECT_EQ(linebuffer.cursor_position(), start);
}

TEST(LineBufferTest, erasesForwardFromWordMiddle) {
    LineBuffer linebuffer {};
    cursor_pos start {1, 6};

    linebuffer.line_start(1);
    linebuffer.set_text("foo bar foo"); // cursor is at "bar"
    linebuffer.cursor_position(start);

    bool status = linebuffer.erase_word_forward();
    EXPECT_TRUE(status);
    EXPECT_EQ(linebuffer.get_text(), "foo b foo");
    EXPECT_EQ(linebuffer.cursor_position(), start);
}

TEST(LineBufferTest, doesntEraseWordForwardWhenBufferEmpty) {
    LineBuffer linebuffer {};
    cursor_pos start {1, 2};

    linebuffer.line_start(1);
    linebuffer.set_text("");
    linebuffer.cursor_position(start);

    bool status = linebuffer.erase_word_forward();
    EXPECT_FALSE(status);
    EXPECT_EQ(linebuffer.cursor_position(), start);
}

TEST(LineBufferTest, yanksLine) {
    LineBuffer linebuffer {};
    cursor_pos start {1, 5};
    const std::string text {"test"};

    linebuffer.line_start(1);
    linebuffer.set_text(text);
    linebuffer.cursor_position(start);

    bool status = linebuffer.erase_to_beginning();

    EXPECT_EQ(linebuffer.get_text(), "");
    EXPECT_EQ(linebuffer.cursor_position(), cursor_pos(1, 1));

    status &= linebuffer.paste();

    EXPECT_TRUE(status);
    EXPECT_EQ(linebuffer.get_text(), text);
    EXPECT_EQ(linebuffer.cursor_position(), start);
}

TEST(LineBufferTest, yanksWord) {
    LineBuffer linebuffer {};
    cursor_pos start {1, 11};
    const std::string text {"test test2"};

    linebuffer.line_start(1);
    linebuffer.set_text(text);
    linebuffer.cursor_position(start);

    bool status = linebuffer.erase_word_backwards();

    EXPECT_EQ(linebuffer.get_text(), "test");
    EXPECT_EQ(linebuffer.cursor_position(), cursor_pos(1, 5));

    status &= linebuffer.paste();

    EXPECT_TRUE(status);
    EXPECT_EQ(linebuffer.get_text(), text);
    EXPECT_EQ(linebuffer.cursor_position(), start);
}
