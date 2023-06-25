#pragma once

#include "driver/gpio.h"

namespace app
{
    class AppLight
    {
    private:
        bool m_initialized;

    public:
        AppLight() : m_initialized(false) {}
        void initialize()
        {
            if (!m_initialized)
            {
                gpio_config_t config_pin2{GPIO_SEL_2, GPIO_MODE_INPUT_OUTPUT, GPIO_PULLUP_DISABLE, GPIO_PULLDOWN_DISABLE, GPIO_INTR_DISABLE};
                gpio_config(&config_pin2);
                m_initialized = true;
            }
            off();
        }
        
        void off()
        {
            gpio_set_level(GPIO_NUM_2, 0);
        }
        void on()
        {
            gpio_set_level(GPIO_NUM_2, 1);
        }
    };
} // namespace app
