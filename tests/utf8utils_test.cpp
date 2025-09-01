#include "linereader/utf8utils.h"
#include <gtest/gtest.h>

TEST(Utf8Test, getsLengthRight) {
    const std::string_view str = "абgdеж";
    const int exp = 6;

    const auto act = utf8_strlen(str);
    EXPECT_EQ(act, exp);
}

TEST(Utf8Test, getsLengthFullAscii) {
    const std::string_view str = "abcdef";
    const int exp = 6;

    const auto act = utf8_strlen(str);
    EXPECT_EQ(act, exp);
}

TEST(Utf8Test, getsLengthFullUtf8) {
    const std::string_view str = "абвгде";
    const int exp = 6;

    const auto act = utf8_strlen(str);
    EXPECT_EQ(act, exp);
}

