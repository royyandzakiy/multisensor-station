#pragma once
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <cstring>

#include <StatefulObject.hpp>
#include <credentials.h>

// ROY: change to WifiState_t

// static EventGroupHandle_t s_wifi_event_group;

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

/* WiFi configuration */
constexpr auto EXAMPLE_ESP_WIFI_SSID = CONFIG_WIFI_SSID;
constexpr auto EXAMPLE_ESP_WIFI_PASS = CONFIG_WIFI_PASSWORD;
constexpr auto EXAMPLE_ESP_MAXIMUM_RETRY = 3;


/* FreeRTOS event group to signal when we are connected */
static EventGroupHandle_t s_wifi_event_group;

/* Event bits */
constexpr auto WIFI_CONNECTED_BIT = BIT0;
constexpr auto WIFI_FAIL_BIT = BIT1;

constexpr const char* WIFI_TAG = "WifiManager";

static int s_retry_num = 0;

// stuff

class WifiManager : public StatefulObject<wifi_event_t> {
public:
    // Get the singleton instance
    static WifiManager& getInstance() {
        static WifiManager instance;
        return instance;
    }

    void init2() {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        esp_netif_create_default_wifi_sta();

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        esp_event_handler_instance_t instance_any_id;
        esp_event_handler_instance_t instance_got_ip;
        // ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &WifiManager::wifiEventHandler, this, &instance_any_id));
        // ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &WifiManager::wifiEventHandler, this, &instance_got_ip));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &WifiManager::event_handler, this, &instance_any_id));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &WifiManager::event_handler, this, &instance_got_ip));

        wifi_config_t wifi_config = {};
        strncpy((char*)wifi_config.sta.ssid, CONFIG_WIFI_SSID, sizeof(wifi_config.sta.ssid));
        strncpy((char*)wifi_config.sta.password, CONFIG_WIFI_PASSWORD, sizeof(wifi_config.sta.password));
        ESP_LOGI(WIFI_TAG, "ROY 0");

        // WifiManager::s_retry_num = 0;

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());

        ESP_LOGI(WIFI_TAG, "ROY 1");

        // EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
        //     WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        //     pdFALSE,
        //     pdFALSE,
        //     portMAX_DELAY);
        
        // if (bits & WIFI_CONNECTED_BIT) {
        //    ESP_LOGI(WIFI_TAG, "connected to ap SSID:%s password:%s",
        //             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
        // } else if (bits & WIFI_FAIL_BIT) {
        //     ESP_LOGI(WIFI_TAG, "Failed to connect to SSID:%s, password:%s",
        //             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
        // } else {
        //     ESP_LOGE(WIFI_TAG, "UNEXPECTED EVENT");
        // }
        
        ESP_LOGI(WIFI_TAG, "ROY 2");

        

        // wifiThread = std::thread(&WifiManager::wifiTask, this);
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

        ESP_LOGI(WIFI_TAG, "WifiManager successfully initialized.");
    }

    bool reconnect() {
        // should define logic here
        // ...
        ESP_LOGI(WIFI_TAG, "ROY:reconnect");
        return true;
    }
    
    bool isConnected() {
        // insert logic here later...
        return true;
    }

    void connect() {
        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

        if (bits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(WIFI_TAG, "Connected to AP SSID:%s password:%s", EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
        } else if (bits & WIFI_FAIL_BIT) {
            ESP_LOGI(WIFI_TAG, "Failed to connect to SSID:%s, password:%s", EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
        } else {
            ESP_LOGE(WIFI_TAG, "UNEXPECTED EVENT");
        }
    }

private:
    WifiManager()
        : StatefulObject<wifi_event_t>("WifiManager", WIFI_EVENT_STA_DISCONNECTED) {
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

    void wifiTask() {
        ESP_LOGI(WIFI_TAG, "ROY: wifiTask");
        while (running) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait_for(lock, std::chrono::seconds(1));
            // Periodically check or perform actions here
            ESP_LOGI(WIFI_TAG, "ROY: wifiTask running");
        }
    }

    static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
        auto& self = WifiManager::getInstance();
        // auto* wifiManager = static_cast<WifiManager*>(arg);

        if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
            self.setState(static_cast<wifi_event_t>(WIFI_EVENT_STA_START));
            esp_wifi_connect();
        } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
            self.setState(static_cast<wifi_event_t>(WIFI_EVENT_STA_DISCONNECTED));
            if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
                esp_wifi_connect();
                s_retry_num++;
                ESP_LOGI(WIFI_TAG, "Retry to connect to the AP");
            } else {
                xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            }
            ESP_LOGI(WIFI_TAG, "Connect to the AP fail");
        } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
            self.setState(static_cast<wifi_event_t>(WIFI_EVENT_STA_CONNECTED));
            auto* event = static_cast<ip_event_got_ip_t*>(event_data);
            ESP_LOGI(WIFI_TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
            s_retry_num = 0;
            xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        }
    }

    esp_event_handler_instance_t instance_any_id{};
    esp_event_handler_instance_t instance_got_ip{};
};