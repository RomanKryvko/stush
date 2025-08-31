#include <linereader/utfstring.h>
#include <gtest/gtest.h>

TEST(UtfStringTest, erasesChars) {
    utf8string str {"абвгде"};
    ASSERT_EQ(str.char_size(), 6);

    str.erase(1, 2);
    EXPECT_EQ(str.stdstr(), "агде");
}

TEST(UtfStringTest, erasesToEnd) {
    utf8string str {"абвгде"};
    ASSERT_EQ(str.char_size(), 6);

    str.stdstr("абвгде");
    str.erase(2);
    EXPECT_EQ(str.stdstr(), "аб");
    EXPECT_EQ(str.char_size(), 2);
}

TEST(UtfStringTest, erasesSingleChars) {
    utf8string str {"абвгде"};
    ASSERT_EQ(str.char_size(), 6);

    str.erase_at(0);
    EXPECT_EQ(str.stdstr(), "бвгде");

    str.erase_at(4);
    EXPECT_EQ(str.stdstr(), "бвгд");
    EXPECT_EQ(str.char_size(), 4);
}

TEST(UtfStringTest, handlesEraseInEmptyString) {
    utf8string str {""};
    ASSERT_EQ(str.char_size(), 0);
    ASSERT_EQ(str.byte_size(), 0);

    EXPECT_THROW(str.erase_at(0), std::out_of_range);
    EXPECT_NO_THROW(str.erase());
    EXPECT_EQ(str.char_size(), 0);
    EXPECT_EQ(str.byte_size(), 0);
}

TEST(UtfStringTest, insertsAsciiChars) {
    utf8string str {"абвгде"};
    ASSERT_EQ(str.char_size(), 6);

    str.insert(1, 'x');
    EXPECT_EQ(str.stdstr(), "аxбвгде");
    EXPECT_EQ(str.char_size(), 7);

    str.insert(0, 'q');
    EXPECT_EQ(str.stdstr(), "qаxбвгде");
    EXPECT_EQ(str.char_size(), 8);

    str.insert(5, 'w');
    EXPECT_EQ(str.stdstr(), "qаxбвwгде");
    EXPECT_EQ(str.char_size(), 9);
}

TEST(UtfStringTest, insertsAsciiString) {
    utf8string str {"абвгде"};
    ASSERT_EQ(str.char_size(), 6);

    str.insert(1, "abc");
    EXPECT_EQ(str.stdstr(), "аabcбвгде");
    EXPECT_EQ(str.char_size(), 9);

    str.insert(0, "xyz");
    EXPECT_EQ(str.stdstr(), "xyzаabcбвгде");
    EXPECT_EQ(str.char_size(), 12);

    str.insert(6, "jkl");
    EXPECT_EQ(str.stdstr(), "xyzаabjklcбвгде");
    EXPECT_EQ(str.char_size(), 15);
}

TEST(UtfStringTest, insertsUtfString) {
    utf8string str {"абвгде"};
    ASSERT_EQ(str.char_size(), 6);

    str.insert(1, "єжз");
    EXPECT_EQ(str.stdstr(), "аєжзбвгде");
    EXPECT_EQ(str.char_size(), 9);

    str.insert(0, "лмн");
    EXPECT_EQ(str.stdstr(), "лмнаєжзбвгде");
    EXPECT_EQ(str.char_size(), 12);

    str.insert(6, "їъп");
    EXPECT_EQ(str.stdstr(), "лмнаєжїъпзбвгде");
    EXPECT_EQ(str.char_size(), 15);
}
