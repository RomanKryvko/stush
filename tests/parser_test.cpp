#include <gtest/gtest.h>
#include <parser.h>

TEST(ParserTest, IgnoresBlankLines) {
    std::vector<std::string> exp {};
    auto blank = sh_tokenize("         ", ' ');
    EXPECT_EQ(exp, blank);
    auto empty = sh_tokenize("", ' ');
    EXPECT_EQ(exp, empty);
}

TEST(ParserTest, LeadingAndTrailingDelimiters) {
    std::vector<std::string> exp {"test", "arg1", "arg2"};
    auto res = sh_tokenize("   test arg1 arg2  ", ' ');
    EXPECT_EQ(exp, res);
}

TEST(ParserTest, SingleWord) {
    std::vector<std::string> exp {"test"};
    auto res = sh_tokenize("test", ' ');
    EXPECT_EQ(exp, res);
}

TEST(ParserTest, NonWhitespaceDelimiter) {
    std::vector<std::string> exp {"test", "arg1", "arg2"};
    auto res = sh_tokenize("test;arg1;arg2", ';');
    EXPECT_EQ(exp, res);
}

TEST(ParserTest, ConsecutiveDelimitersAreIgnored) {
    std::vector<std::string> exp {"a", "b", "c"};
    auto res = sh_tokenize("a   b    c", ' ');
    EXPECT_EQ(exp, res);
}
