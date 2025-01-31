#pragma once

#include <string>
#include <memory>
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <lwip/err.h>
#include <lwip/sys.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string.h>

#include <../../src/credentials.h>
#include <StatefulObject.hpp>

/* WiFi configuration */
constexpr auto WIFI_SSID = CONFIG_WIFI_SSID;
constexpr auto WIFI_PASS = CONFIG_WIFI_PASSWORD;
constexpr auto WIFI_RECONNECT_ATTEMPT_MAXIMUM_RETRY = 5;

enum class wifiState_t {
    NOT_INITIALIZED,
    DISCONNECTED,
    CONNECTING,
    CONNECTED
};

class WifiManager : public StatefulObject<wifiState_t> {
public:
    // Get the singleton instance
    static WifiManager& getInstance() {
        static WifiManager instance;
        return instance;
    }

    void init();

    bool isConnected() {
        return getState() == wifiState_t::CONNECTED;
    }

private:
    void reconnectTask();

    WifiManager() : StatefulObject<wifiState_t>("WifiManager", wifiState_t::NOT_INITIALIZED) {
        // Start the reconnectTask in a separate thread
        std::thread(&WifiManager::reconnectTask, this).detach();
    }

    ~WifiManager() {
        if (wifiCheckThread.joinable()) {
            wifiCheckThread.join();
        }
    }

    void checkWifiSignalStrength();

    void wifiCheckTask();

    static void eventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

    std::mutex mtx;
    std::condition_variable cv;
    std::thread wifiCheckThread;

    static constexpr const char* TAG = "WifiManager";
    static int wifi_reconnect_retry_count;
    static EventGroupHandle_t s_wifi_event_group;
    static constexpr int WIFI_CONNECTED_BIT = BIT0;
    static constexpr int WIFI_FAIL_BIT = BIT1;    

    esp_event_handler_instance_t instance_any_id{};
    esp_event_handler_instance_t instance_got_ip{};
};