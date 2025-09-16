#include "cmd/cmd.h"
#include "cmd/expansion.h"
#include "cmd/variable.h"
#include "parser.h"
#include <cstdlib>
#include <gtest/gtest.h>
#include <string>

TEST(VarExpansion, expandsSingleShellVars) {
    var::set_var("USERHOME", "/home/user1");
    ASSERT_TRUE(var::is_set("USERHOME"));
    ASSERT_EQ(var::get_var("USERHOME"), "/home/user1");

    std::string varstr {"$USERHOME"};
    const std::string exp {"/home/user1"};

    expand_word(varstr);

    EXPECT_EQ(varstr, exp);
}

TEST(VarExpansion, expandsAllShellVarsInString) {
    var::set_var("USERHOME", "/home/user1");
    ASSERT_TRUE(var::is_set("USERHOME"));
    ASSERT_EQ(var::get_var("USERHOME"), "/home/user1");
    var::set_var("DIR", "foo");
    var::set_var("NAME", "bar");
    var::set_var("SMTH", "something");

    std::string varstr {"$USERHOME/$DIR $NAME;$SMTH*"};
    const std::string exp {"/home/user1/foo bar;something*"};

    expand_word(varstr);

    EXPECT_EQ(varstr, exp);
}

TEST(VarExpansion, doesntExpandEscapedVars) {
    var::set_var("USERHOME", "/home/user1");
    var::set_var("DIR", "foo");
    var::set_var("NAME", "bar");
    var::set_var("SMTH", "something");

    std::string varstr {"\\$USERHOME/$DIR \\$NAME;$SMTH*"};
    const std::string exp {"$USERHOME/foo $NAME;something*"};

    expand_word(varstr);

    EXPECT_EQ(varstr, exp);
}

TEST(TildeExpansion, expandsDefaultHome) {
    const char* home {getenv("HOME")};
    std::string tildestr {"~"};
    const std::string exp {home};

    expand_tilde(tildestr);

    EXPECT_EQ(tildestr, exp);
}

TEST(TildeExpansion, expandsUserHome) {
    std::string tildestr {"~nobody/"};
    const std::string exp {"/"};

    expand_tilde(tildestr);

    EXPECT_EQ(tildestr, exp);

    tildestr = "~nobody";
    expand_tilde(tildestr);
    EXPECT_EQ(tildestr, "/");

    tildestr = "~root/";
    const std::string exp2 {"/root/"};

    expand_tilde(tildestr);

    EXPECT_EQ(tildestr, exp2);
}

TEST(TildeExpansion, doesntExpandEscaped) {
    std::string tildestr {"\\~nobody/"};
    const std::string exp {"\\~nobody/"};
    expand_tilde(tildestr);
    EXPECT_EQ(tildestr, exp);
}

TEST(ExpansionIntegrationTest, preservesTextInQuotes) {
    var::set_var("DIR", "location");
    const std::string input {"ls $DIR   ' enclosed args   ' some/~/location     \"enclosed '2'\" "};
    const args_container exp {"ls", "location", "' enclosed args   '",
        "some/~/location", "\"enclosed '2'\""};

    auto act {sh_tokenize(input, ' ')};
    for (auto& word : act) {
        expand_word(word);
        expand_tilde(word);
    }
    EXPECT_EQ(act, exp);
}

TEST(ExpansionIntegrationTest, emptyInputGivesNoArgs) {
    const std::string input {""};
    const args_container exp {};
    auto act {sh_tokenize(input, ' ')};
    for (auto& word : act) {
        expand_word(word);
        expand_tilde(word);
    }
    EXPECT_EQ(act, exp);
}

TEST(ExpansionIntegrationTest, emptyQuotesGiveEmptyArgs) {
    const std::string input {"ls ''   '' \"    \" 'word' "};
    const args_container exp {"ls", "''", "''", "\"    \"", "'word'"};
    auto act {sh_tokenize(input, ' ')};
    for (auto& word : act) {
        expand_word(word);
        expand_tilde(word);
    }
    EXPECT_EQ(act, exp);
}
