#pragma once
#include <iostream>
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <esp_ota_ops.h>
#include <esp_http_client.h>
#include <esp_https_ota.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <StatefulObject.hpp>

#define BUTTON_GPIO GPIO_NUM_0
#define TAG "OTA_UPDATE"

enum class OtaState {
    Idle,           // Device is idle, no OTA in progress
    Checking,       // Checking for OTA updates
    Downloading,    // Downloading the OTA firmware
    Applying,       // Applying the OTA firmware
    Success,        // OTA update successful
    Failed          // OTA update failed
};

class OtaManager : public StatefulObject<OtaState> {
public:
    // Get the singleton instance
    static OtaManager& getInstance() {
        static OtaManager instance("OTAManager");
        return instance;
    }

    void start() {
        // Start the button monitoring thread
        std::thread buttonThread(&OtaManager::monitorButton, this);
        buttonThread.detach();

        // Start the OTA check thread
        std::thread otaThread(&OtaManager::checkForOtaUpdates, this);
        otaThread.detach();

        // Main application loop
        while (true) {
            // Simulate normal operation
            ESP_LOGI(TAG, "Main application running...");
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

private:
    OtaManager(const std::string& id)
        : StatefulObject<OtaState>(id, OtaState::Idle) {
        gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
        gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);
    }

    void monitorButton() {
        while (true) {
            if (gpio_get_level(BUTTON_GPIO) == 0) { // Button pressed
                ESP_LOGI(TAG, "Button pressed, entering OTA mode...");
                enterOtaMode();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void enterOtaMode() {
        if (getState() != OtaState::Idle) {
            ESP_LOGW(TAG, "OTA already in progress.");
            return;
        }

        performOtaUpdate();
    }

    void checkForOtaUpdates() {
        while (true) {
            if (getState() == OtaState::Idle) {
                // Simulate checking for OTA updates from ThingsBoard
                bool otaAvailable = checkOtaAvailability();
                if (otaAvailable) {
                    ESP_LOGI(TAG, "OTA update available, starting update...");
                    enterOtaMode();
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(10)); // Check every 10 seconds
        }
    }

    bool checkOtaAvailability() {
        // Simulate checking with ThingsBoard
        // Replace with actual logic to query ThingsBoard
        return false; // Return true if OTA is available
    }

    void performOtaUpdate() {
        setState(OtaState::Checking);

        // Simulate checking for updates
        std::this_thread::sleep_for(std::chrono::seconds(2));
        setState(OtaState::Downloading);

        // Perform the OTA update
        esp_http_client_config_t config = {
            .url = "https://your.thingsboard.server/firmware.bin",
            .cert_pem = nullptr, // Add certificate if using HTTPS
        };

        esp_err_t ret = esp_https_ota(&config);
        if (ret == ESP_OK) {
            setState(OtaState::Success);
            ESP_LOGI(TAG, "OTA update successful, restarting...");
        } else {
            setState(OtaState::Failed);
            ESP_LOGE(TAG, "OTA update failed, restarting...");
        }

        // Restart the device
        esp_restart();
    }
};