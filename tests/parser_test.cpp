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

TEST(ParserTest, QuotesGroupWords) {
    args_container exp {"a", "\"b   c   d\"", "e"};
    auto res = sh_tokenize("a  \"b   c   d\"    e   ", ' ');
    EXPECT_EQ(exp, res);
}

TEST(ParserTest, DifferentQuoteMarksAreIgnored) {
    args_container exp {"a", "\"b   'c '  d\"", "e"};
    auto res = sh_tokenize("a  \"b   'c '  d\"    e   ", ' ');
    EXPECT_EQ(exp, res);
}

TEST(ParserTest, EmptyQuotesBecomeEmptyArg) {
    args_container exp {"a", "\"\"", "b"};
    auto res = sh_tokenize("a \"\" b", ' ');
    EXPECT_EQ(exp, res);
}

TEST(ParserTest, QuotesPreserveSpaces) {
    args_container exp {"a", "\"   \"", "b"};
    auto res = sh_tokenize("a \"   \" b", ' ');
    EXPECT_EQ(exp, res);
}

TEST(ParserTest, NestedSingleQuotesInsideDoubleQuotes) {
    args_container exp {"cmd", "\"arg 'with single quotes'\"", "end"};
    auto res = sh_tokenize("cmd \"arg 'with single quotes'\" end", ' ');
    EXPECT_EQ(exp, res);
}

TEST(ParserTest, NestedDoubleQuotesInsideSingleQuotes) {
    args_container exp {"cmd", "'arg \"with double quotes\"'", "end"};
    auto res = sh_tokenize("cmd 'arg \"with double quotes\"' end", ' ');
    EXPECT_EQ(exp, res);
}

TEST(ParserTest, QuotesAroundSingleWord) {
    args_container exp {"\"word\""};
    auto res = sh_tokenize("\"word\"", ' ');
    EXPECT_EQ(exp, res);
}

TEST(ParserTest, EscapedQuotesAroundSingleWord) {
    args_container exp {R"(\"word\")"};
    auto res = sh_tokenize(R"(\"word\")", ' ');
    EXPECT_EQ(exp, res);
}

TEST(ParserTest, EscapedSpaces) {
    args_container exp {R"(a\ sentence\ with\ escaped\ words)"};
    auto res = sh_tokenize(R"(a\ sentence\ with\ escaped\ words)", ' ');
    EXPECT_EQ(exp, res);
}

TEST(ParserTest, SplitsOutCommandSeparators) {
    args_container exp {"echo", ";", "exit"};
    auto res = sh_tokenize("echo;exit", ' ');
    EXPECT_EQ(exp, res);
}

TEST(ParserTest, SplitsOutDoubleCommandSeparators) {
    args_container exp {"echo", ";", "exit", "&&", "echo", "|", "wc", "-l",
        "||", "exit"};
    auto res = sh_tokenize("echo;exit&&echo| wc -l || exit", ' ');
    EXPECT_EQ(exp, res);
}

TEST(ParserTest, HandlesCommandSeparatorsAtTheEnd) {
    args_container exp {"echo", "&&", "exit", ";"};
    auto res = sh_tokenize("  echo&&  exit;", ' ');
    EXPECT_EQ(exp, res);

    exp = {"echo", "&&", "exit", ";", ";", ";", "&&"};
    res = sh_tokenize("  echo&&  exit;;;     &&       ", ' ');
    EXPECT_EQ(exp, res);

    exp = {"echo", "&&", "exit", "||"};
    res = sh_tokenize("  echo&&  exit||", ' ');
    EXPECT_EQ(exp, res);
}
