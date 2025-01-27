#include <gtest/gtest.h>
#include "AppLight.hpp"
#include "driver/gpio.h"

extern "C" void app_main()
{
    ::testing::InitGoogleTest();
    // ::testing::GTEST_FLAG(filter) = "LightTest";
    ::testing::GTEST_FLAG(filter) = "LightTest.should_set_gpio_2_to_high_when_applight_on";
    RUN_ALL_TESTS();
}