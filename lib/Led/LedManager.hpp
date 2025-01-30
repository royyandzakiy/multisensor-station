#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <functional>
#include <driver/gpio.h> // ESP-IDF GPIO driver
#include <esp_log.h> // ESP-IDF Log driver
#include <EventLogger.hpp> // ESP-IDF Log driver

class LedManager {
public:
    enum class LEDColor {
        RED,
        GREEN,
        BLUE
    };

    enum class LEDMode {
        OFF,
        ON,
        BLINK
    };

    // Get the singleton instance
    static LedManager& getInstance() {
        static LedManager instance;
        return instance;
    }

    // Initialize GPIOs for LEDs
    void init() {
        // Configure GPIOs for LEDs
        gpio_config_t io_conf;
        io_conf.intr_type = GPIO_INTR_DISABLE; // Disable interrupt
        io_conf.mode = GPIO_MODE_OUTPUT;      // Set as output mode
        io_conf.pin_bit_mask = (1ULL << redPin) | (1ULL << greenPin) | (1ULL << bluePin);
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // Disable pull-down
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;     // Disable pull-up
        gpio_config(&io_conf);

        ESP_LOGI(TAG, "LED GPIOs initialized");
    }

    // Set the mode (off, on, blink) of an LED
    void setLEDMode(LEDColor color, LEDMode mode, int intervalMs = 1000) {
        std::lock_guard<std::mutex> lock(mtx);
        ledModes[color] = mode;
        ledIntervals[color] = intervalMs;
        ESP_LOGI(TAG, "Set %s LED to %s with interval %d ms", getColorName(color).c_str(), getModeName(mode).c_str(), intervalMs);

        // Immediately update the LED state
        updateLEDState(color);
    }

private:
    LedManager() {
        // Register a callback with EventLogger
        EventLogger::getInstance().registerCallback(
            [this](const std::string& id, const std::string& state) {
                processLog(id, state);
            }
        );

        // Start the LED blinking task
        blinkingThread = std::thread(&LedManager::blinkingTask, this);
    }

    ~LedManager() {
        running = false;
        if (blinkingThread.joinable()) {
            blinkingThread.join();
        }
    }

    // Delete copy constructor and assignment operator
    LedManager(const LedManager&) = delete;
    LedManager& operator=(const LedManager&) = delete;

    // GPIO pins for LEDs
    const gpio_num_t redPin = GPIO_NUM_17;    // GPIO 17
    const gpio_num_t greenPin = GPIO_NUM_18;  // GPIO 18
    const gpio_num_t bluePin = GPIO_NUM_19;   // GPIO 19

    std::thread blinkingThread;
    std::mutex mtx;
    std::atomic<bool> running{true};

    // LED modes (off, on, blink)
    std::unordered_map<LEDColor, LEDMode> ledModes = {
        {LEDColor::RED, LEDMode::OFF},
        {LEDColor::GREEN, LEDMode::OFF},
        {LEDColor::BLUE, LEDMode::OFF}
    };

    // LED blinking intervals (in milliseconds)
    std::unordered_map<LEDColor, int> ledIntervals = {
        {LEDColor::RED, 1000},
        {LEDColor::GREEN, 1000},
        {LEDColor::BLUE, 1000}
    };

    static constexpr const char* TAG = "LedManager";

    // Process a log entry and update LED modes
    void processLog(const std::string& id, const std::string& state) {
        std::lock_guard<std::mutex> lock(mtx);

        // Define LED behavior based on id and state
        if (id == "WifiManager") {
            if (state == "CONNECTED") {
                setLEDMode(LEDColor::GREEN, LEDMode::BLINK, 500); // Blink green LED every 500ms
            } else if (state == "DISCONNECTED") {
                setLEDMode(LEDColor::GREEN, LEDMode::OFF);
            }
        } else if (id == "MqttManager") {
            if (state == "CONNECTED") {
                setLEDMode(LEDColor::BLUE, LEDMode::BLINK, 1000); // Blink blue LED every 1000ms
            } else if (state == "DISCONNECTED") {
                setLEDMode(LEDColor::BLUE, LEDMode::OFF);
            }
        } else if (id == "Error") {
            setLEDMode(LEDColor::RED, LEDMode::BLINK, 200); // Blink red LED rapidly (200ms) for errors
        }
    }

    // Update the physical state of an LED based on its mode
    void updateLEDState(LEDColor color) {
        gpio_num_t pin = getPinForColor(color);
        LEDMode mode = ledModes[color];

        switch (mode) {
            case LEDMode::OFF:
                gpio_set_level(pin, 0);
                break;
            case LEDMode::ON:
                gpio_set_level(pin, 1);
                break;
            case LEDMode::BLINK:
                // Blinking is handled in the blinkingTask
                break;
        }
    }

    // Task to handle LED blinking
    void blinkingTask() {
        while (running) {
            std::unique_lock<std::mutex> lock(mtx);
            for (const auto& [color, mode] : ledModes) {
                if (mode == LEDMode::BLINK) {
                    gpio_num_t pin = getPinForColor(color);
                    gpio_set_level(pin, !gpio_get_level(pin)); // Toggle LED state
                }
            }
            lock.unlock();

            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Adjust timing as needed
        }
    }

    // Get the GPIO pin for an LED color
    gpio_num_t getPinForColor(LEDColor color) const {
        switch (color) {
            case LEDColor::RED: return redPin;
            case LEDColor::GREEN: return greenPin;
            case LEDColor::BLUE: return bluePin;
            default: return GPIO_NUM_NC; // Not connected
        }
    }

    // Get the name of an LED color
    std::string getColorName(LEDColor color) const {
        switch (color) {
            case LEDColor::RED: return "Red";
            case LEDColor::GREEN: return "Green";
            case LEDColor::BLUE: return "Blue";
            default: return "Unknown";
        }
    }

    // Get the name of an LED mode
    std::string getModeName(LEDMode mode) const {
        switch (mode) {
            case LEDMode::OFF: return "OFF";
            case LEDMode::ON: return "ON";
            case LEDMode::BLINK: return "BLINK";
            default: return "UNKNOWN";
        }
    }
};