#include <gtest/gtest.h>

TEST(BasicTest, MathCheck) {
    EXPECT_EQ(1 + 1, 2);
}

TEST(BasicTest, StringCheck) {
    std::string hello = "Hello, PlatformIO!";
    EXPECT_EQ(hello, "Hello, PlatformIO!");
}