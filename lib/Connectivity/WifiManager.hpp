#pragma once
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <StatefulObject.hpp>
#include <credentials.h>
#include <cstring>

class WifiManager : public StatefulObject<std::string> {
public:
    // Get the singleton instance
    static WifiManager& getInstance() {
        static WifiManager instance;
        return instance;
    }

    void init() {
        esp_netif_init();
        esp_event_loop_create_default();
        esp_netif_create_default_wifi_sta();

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        esp_wifi_init(&cfg);

        esp_event_handler_instance_t instance_any_id;
        esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &WifiManager::wifiEventHandler, this, &instance_any_id);
        esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &WifiManager::wifiEventHandler, this, &instance_any_id);

        wifi_config_t wifi_config = {};
        strncpy((char*)wifi_config.sta.ssid, CONFIG_WIFI_SSID, sizeof(wifi_config.sta.ssid));
        strncpy((char*)wifi_config.sta.password, CONFIG_WIFI_PASSWORD, sizeof(wifi_config.sta.password));

        esp_wifi_set_mode(WIFI_MODE_STA);
        esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
        esp_wifi_start();

        wifiThread = std::thread(&WifiManager::wifiTask, this);
    }

    bool reconnect() {
        // should define logic here
        // ...
        return true;
    }
    
    bool isConnected() {
        // insert logic here later...
        return true;
    }

private:
    WifiManager()
        : StatefulObject<std::string>("WifiManager", "DISCONNECTED") {
    }

    ~WifiManager() {
        stop();
    }

    std::thread wifiThread;
    std::mutex mtx;
    std::condition_variable cv;
    bool running = true;

    void stop() {
        running = false;
        cv.notify_all();
        if (wifiThread.joinable()) {
            wifiThread.join();
        }
    }

    static void wifiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
        auto* self = static_cast<WifiManager*>(arg);
        if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
            esp_wifi_connect();
            self->setState("CONNECTING");
        } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
            self->setState("DISCONNECTED");
        } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
            self->setState("CONNECTED");
        }
    }

    void wifiTask() {
        while (running) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait_for(lock, std::chrono::seconds(1));
            // Periodically check or perform actions here
        }
    }
};