#include <gtest/gtest.h>

TEST(BasicTest, should_equal_2_if_one_plus_one) {
    int actual = 1 + 1;
    int expected = 2;
    EXPECT_EQ(actual, expected);
}

TEST(BasicTest, should_have_string_to_match) {
    std::string hello = "Hello, PlatformIO!";
    EXPECT_EQ(hello, "Hello, PlatformIO!");
}
