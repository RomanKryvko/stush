#include <algorithm>
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

TEST(UtfStringTest, insertsUtfStringObject) {
    utf8string str {"абвгде"};
    ASSERT_EQ(str.char_size(), 6);

    utf8string utf_str1 {"єжз"};
    str.insert_utf8(1, utf_str1);
    EXPECT_EQ(str.stdstr(), "аєжзбвгде");
    EXPECT_EQ(str.char_size(), 9);

    utf8string utf_str2 {"лмн"};
    str.insert_utf8(0, utf_str2);
    EXPECT_EQ(str.stdstr(), "лмнаєжзбвгде");
    EXPECT_EQ(str.char_size(), 12);

    utf8string utf_str3 {"їъп"};
    str.insert_utf8(6, utf_str3);
    EXPECT_EQ(str.stdstr(), "лмнаєжїъпзбвгде");
    EXPECT_EQ(str.char_size(), 15);
}

TEST(UtfStringTest, utfStringRoundTrip) {
    utf8string str {"абвгде"};
    ASSERT_EQ(str.char_size(), 6);

    str.insert(1, "єжз");
    EXPECT_EQ(str.stdstr(), "аєжзбвгде");
    EXPECT_EQ(str.char_size(), 9);

    str.insert(0, "лмн");
    EXPECT_EQ(str.stdstr(), "лмнаєжзбвгде");
    EXPECT_EQ(str.char_size(), 12);

    str.erase(0, 10);
    EXPECT_EQ(str.stdstr(), "де");
    EXPECT_EQ(str.char_size(), 2);

    str.insert(1, "їъп");
    EXPECT_EQ(str.stdstr(), "дїъпе");
    EXPECT_EQ(str.char_size(), 5);

    str.erase(0, 1);
    EXPECT_EQ(str.stdstr(), "їъпе");
    EXPECT_EQ(str.char_size(), 4);
}

TEST(Utf8String, equalsAtWorks) {
    utf8string s("hello Ф world");

    EXPECT_EQ(s.at(0), 'h');
    EXPECT_NE(s.at(0), 'a');
    EXPECT_EQ(s.at(4), 'o');
    EXPECT_EQ(s.at(6), U'Ф');
}

TEST(Utf8String, appendWorks) {
    utf8string s("h");

    s += 'e';
    EXPECT_EQ(s.stdstr(), "he");

    s.append("llo");
    EXPECT_EQ(s.stdstr(), "hello");

    s += U'ф';
    EXPECT_EQ(s.stdstr(), "helloф");
}

TEST(Utf8StringFindTest, FindFirstOfAscii) {
    utf8string s("hello world");
    EXPECT_EQ(s.find_first_of('o'), 4);
    EXPECT_EQ(s.find_first_of('o', 5), 7);
    EXPECT_EQ(s.find_first_of('z'), std::string::npos);
}

TEST(Utf8StringFindTest, FindLastOfAscii) {
    utf8string s("banana");
    EXPECT_EQ(s.find_last_of('a'), 5);
    EXPECT_EQ(s.find_last_of('a', 4), 3);
    EXPECT_EQ(s.find_last_of('z'), std::string::npos);
}

TEST(Utf8StringFindTest, FindFirstNotOfAscii) {
    utf8string s("   abc");
    EXPECT_EQ(s.find_first_not_of(' '), 3);
    EXPECT_EQ(s.find_first_not_of(' ', 4), 4);
    EXPECT_EQ(s.find_first_not_of(' ' , 10), std::string::npos);
}

TEST(Utf8StringFindTest, FindLastNotOfAscii) {
    utf8string s("xxxooo");
    EXPECT_EQ(s.find_last_not_of('o'), 2);
    EXPECT_EQ(s.find_last_not_of('x'), 5);
    EXPECT_EQ(s.find_last_not_of('z'), 5);  // everything is not 'z'
}

TEST(Utf8StringFindTest, FindFirstOfStringView) {
    utf8string s("abcdef");
    EXPECT_EQ(s.find_first_of("de"), 3);
    EXPECT_EQ(s.find_first_of("de", 4), 4);
    EXPECT_EQ(s.find_first_of("xyz"), std::string::npos);
}

TEST(Utf8StringFindTest, FindLastOfStringView) {
    utf8string s("abcdefabc");
    EXPECT_EQ(s.find_last_of("abc"), 8);
    EXPECT_EQ(s.find_last_of("abc", 6), 6);
    EXPECT_EQ(s.find_last_of("xyz"), std::string::npos);
}

TEST(Utf8StringFindTest, FindFirstNotOfStringView) {
    utf8string s("aaabbbc");
    EXPECT_EQ(s.find_first_not_of("ab"), 6);  // 'c'
    EXPECT_EQ(s.find_first_not_of("abc"), std::string::npos);
}

TEST(Utf8StringFindTest, FindLastNotOfStringView) {
    utf8string s("abccccc");
    EXPECT_EQ(s.find_last_not_of("c"), 1); // last not 'c'
    EXPECT_EQ(s.find_last_not_of("abc"), std::string::npos);
}

TEST(Utf8StringFindTest, HandlesUnicodeCharacters) {
    utf8string s("héllo😀world");

    // UTF-8 aware tests
    EXPECT_EQ(s.find_first_of('h'), 0);
    EXPECT_EQ(s.find_first_of("😀"), 5); // emoji

    // Find not of
    EXPECT_EQ(s.find_first_not_of("héllo😀"), 6); // 'w'
    EXPECT_EQ(s.find_last_not_of("world😀hélo"), std::string::npos);
}

TEST(Utf8StringFindTest, EmptyStringCases) {
    utf8string empty("");
    EXPECT_EQ(empty.find_first_of('a'), std::string::npos);
    EXPECT_EQ(empty.find_last_of('a'), std::string::npos);
    EXPECT_EQ(empty.find_first_not_of('a'), std::string::npos);
    EXPECT_EQ(empty.find_last_not_of('a'), std::string::npos);
}

TEST(Utf8StringIterTest, satisfiesRangeFor) {

    utf8string exp {"абвгде"};

    utf8string act {};
    for (const auto c : exp) {
        act += c;
    }
    EXPECT_EQ(exp, act);
}

TEST(Utf8StringIterTest, satisfiesFind) {
    utf8string str {"абвгде"};

    auto act = std::find(str.cbegin(), str.cend(), U'в');
    EXPECT_EQ(*act, U'в');
    EXPECT_EQ(*(++act), U'г');
}
