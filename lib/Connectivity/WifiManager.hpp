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

#include <StatefulObject.hpp>
#include <credentials.h>

/* WiFi configuration */
constexpr auto ESP_WIFI_SSID = CONFIG_WIFI_SSID;
constexpr auto ESP_WIFI_PASS = CONFIG_WIFI_PASSWORD;
constexpr auto ESP_WIFI_RECONNECT_ATTEMPT_MAXIMUM_RETRY = 5;

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

    void init() {
        s_wifi_event_group = xEventGroupCreate();
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        esp_netif_create_default_wifi_sta();

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &WifiManager::event_handler, nullptr, &instance_any_id));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &WifiManager::event_handler, nullptr, &instance_got_ip));

        wifi_config_t wifi_config = {};
        strncpy((char*)wifi_config.sta.ssid, CONFIG_WIFI_SSID, sizeof(wifi_config.sta.ssid));
        strncpy((char*)wifi_config.sta.password, CONFIG_WIFI_PASSWORD, sizeof(wifi_config.sta.ssid));

        ESP_LOGI(WIFI_TAG, "wifi_config.sta.ssid: %s", wifi_config.sta.ssid);
        ESP_LOGI(WIFI_TAG, "wifi_config.sta.password: %s", wifi_config.sta.password);

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());

        // taskThread = std::thread(&WifiManager::task, this);
        // std::thread(&WifiManager::reconnectTask, this).detach();
        // taskThread = std::thread(&WifiManager::reconnectTask, this);

        ESP_LOGI(WIFI_TAG, "WifiManager successfully initialized.");
    }

private:
    void reconnectTask() {
        ESP_LOGI(WIFI_TAG, "reconnectTask Started");

        while (true) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return getState() == wifiState_t::DISCONNECTED; });

            ESP_LOGI(WIFI_TAG, "Attempting to reconnect to Wi-Fi...");

            if (WifiManager::wifi_reconnect_count < ESP_WIFI_RECONNECT_ATTEMPT_MAXIMUM_RETRY) {
                ESP_LOGI(WIFI_TAG, "Retry to connect to the AP");
                ESP_ERROR_CHECK(esp_wifi_connect());    
                wifi_reconnect_count++;
            } else {
                xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
                ESP_LOGI(WIFI_TAG, "Max reconnect attempt exceeded, will restart");
                esp_restart();
            }

            // Wait until the connection is restored for a certain time, if still disconnected, go to sleep. else continue
            cv.wait(lock, [this] { return getState() == wifiState_t::CONNECTED; }); // BUG: change this

            ESP_LOGI(WIFI_TAG, "Connection restored. ReconnectTask will now wait for the next disconnect event.");
        }
    }

    WifiManager() : StatefulObject<wifiState_t>("WifiManager", wifiState_t::NOT_INITIALIZED) {
        // Start the reconnectTask in a separate thread
        std::thread(&WifiManager::reconnectTask, this).detach();
    }

    ~WifiManager() {
        stop();
    }

    void stop() {
        running = false;
        cv.notify_all();
        if (taskThread.joinable()) {
            taskThread.join();
        }
    }

    void check_wifi_signal_strength() {
        wifi_ap_record_t ap_info;
        esp_err_t err = esp_wifi_sta_get_ap_info(&ap_info);

        if (err == ESP_OK) {
            ESP_LOGI(WIFI_TAG, "SSID: %s", ap_info.ssid);
            ESP_LOGI(WIFI_TAG, "RSSI: %d dBm", ap_info.rssi);
            ESP_LOGI(WIFI_TAG, "Channel: %d", ap_info.primary);
            ESP_LOGI(WIFI_TAG, "Authmode: %d", ap_info.authmode);
        } else {
            ESP_LOGE(WIFI_TAG, "Failed to get AP info: %s", esp_err_to_name(err));
        }
    }

    void task() {
        ESP_LOGI(WIFI_TAG, "task Started");
        while (running) {
            std::unique_lock<std::mutex> lock(mtx);
            
            // Periodically check or perform actions here
            check_wifi_signal_strength();

            ESP_LOGI(WIFI_TAG, "task running");
            cv.wait_for(lock, std::chrono::seconds(5));
        }
    }

    static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
        auto& self = WifiManager::getInstance();

        if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
            self.setState(wifiState_t::CONNECTING);
            ESP_ERROR_CHECK(esp_wifi_connect());
        } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
            self.setState(wifiState_t::DISCONNECTED);
            ESP_LOGI(WIFI_TAG, "Connect to the AP fail/disconnected");

            // Notify the reconnectTask to start
            self.cv.notify_all();
        } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
            self.setState(wifiState_t::CONNECTED);

            auto* event = static_cast<ip_event_got_ip_t*>(event_data);
            ESP_LOGI(WIFI_TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
            wifi_reconnect_count = 0;
            xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);

            // Notify the reconnectTask that the connection is restored
            self.cv.notify_all();
        }
    }

    std::mutex mtx;
    std::condition_variable cv;
    bool running = true;
    static constexpr const char* WIFI_TAG = "WifiManager";
    static int wifi_reconnect_count;
    static EventGroupHandle_t s_wifi_event_group;
    static constexpr int WIFI_CONNECTED_BIT = BIT0;
    static constexpr int WIFI_FAIL_BIT = BIT1;    

    std::thread taskThread;

    esp_event_handler_instance_t instance_any_id{};
    esp_event_handler_instance_t instance_got_ip{};
};

int WifiManager::wifi_reconnect_count = 0;
EventGroupHandle_t WifiManager::s_wifi_event_group = xEventGroupCreate();