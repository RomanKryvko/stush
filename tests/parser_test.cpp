#include <gtest/gtest.h>
#include <parser.h>

TEST(ParserTest, IgnoresBlankLines) {
    args_container exp {};
    auto blank = sh_tokenize("         ", ' ');
    EXPECT_EQ(exp, blank);
    auto empty = sh_tokenize("", ' ');
    EXPECT_EQ(exp, empty);
}

TEST(ParserTest, LeadingAndTrailingDelimiters) {
    args_container exp {"test", "arg1", "arg2"};
    auto res = sh_tokenize("   test arg1 arg2  ", ' ');
    EXPECT_EQ(exp, res);
}

TEST(ParserTest, SingleWord) {
    args_container exp {"test"};
    auto res = sh_tokenize("test", ' ');
    EXPECT_EQ(exp, res);
}

TEST(ParserTest, NonWhitespaceDelimiter) {
    args_container exp {"test", "arg1", "arg2"};
    auto res = sh_tokenize("test;arg1;arg2", ';');
    EXPECT_EQ(exp, res);
}

TEST(ParserTest, ConsecutiveDelimitersAreIgnored) {
    args_container exp {"a", "b", "c"};
    auto res = sh_tokenize("a   b    c", ' ');
    EXPECT_EQ(exp, res);
}
