#include "cmd/expansion.h"
#include "cmd/variable.h"
#include <cstdlib>
#include <gtest/gtest.h>
#include <string>

TEST(VarExpansion, expandsSingleShellVars) {
    var::set_var("USERHOME", "/home/user1");
    ASSERT_TRUE(var::is_set("USERHOME"));
    ASSERT_EQ(var::get_var("USERHOME"), "/home/user1");

    std::string varstr {"$USERHOME"};
    const std::string exp {"/home/user1"};

    expand_all_variables(varstr);

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

    expand_all_variables(varstr);

    EXPECT_EQ(varstr, exp);
}

TEST(VarExpansion, doesntExpandEscapedVars) {
    var::set_var("USERHOME", "/home/user1");
    var::set_var("DIR", "foo");
    var::set_var("NAME", "bar");
    var::set_var("SMTH", "something");

    std::string varstr {"\\$USERHOME/$DIR \\$NAME;$SMTH*"};
    const std::string exp {"$USERHOME/foo $NAME;something*"};

    expand_all_variables(varstr);

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
