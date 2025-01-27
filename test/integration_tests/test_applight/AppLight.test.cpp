#include <gtest/gtest.h>
#include "AppLight.hpp"
#include "driver/gpio.h"

class LightTest : public ::testing::Test
{
protected:
    static AppLight light; // static because only need 1 instance for testing
    LightTest()
    {
        light.initialize();
    }
};
AppLight LightTest::light;

TEST_F(LightTest, should_set_gpio_2_to_high_when_applight_on)
{
    light.on();
    ASSERT_GT(gpio_get_level(GPIO_NUM_2), 0); // greater than 0
}

TEST_F(LightTest, should_set_gpio_2_to_low_when_applight_off)
{
    light.off();
    ASSERT_EQ(gpio_get_level(GPIO_NUM_2), 0); // equal to 0
}