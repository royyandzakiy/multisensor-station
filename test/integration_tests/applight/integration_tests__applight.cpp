#include <gtest/gtest.h>
#include "AppLight.hpp"
#include "driver/gpio.h"

extern "C" void app_main()
{
    ::testing::InitGoogleTest();
    RUN_ALL_TESTS();
}