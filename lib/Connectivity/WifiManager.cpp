#include <WifiManager.hpp>

int WifiManager::wifi_reconnect_retry_count = 0;
EventGroupHandle_t WifiManager::s_wifi_event_group = xEventGroupCreate();

void WifiManager::init() {
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t* sta_netif = esp_netif_create_default_wifi_sta();
    if (!sta_netif) {
        ESP_LOGE(TAG, "Failed to create default WiFi STA interface");
        return;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &WifiManager::eventHandler, nullptr, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &WifiManager::eventHandler, nullptr, &instance_got_ip));

    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.sta.ssid, CONFIG_WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, CONFIG_WIFI_PASSWORD, sizeof(wifi_config.sta.ssid));

    ESP_LOGI(TAG, "wifi_config.sta.ssid: %s", wifi_config.sta.ssid);
    ESP_LOGI(TAG, "wifi_config.sta.password: %s", wifi_config.sta.password);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

#ifdef DEBUG_MODE
    wifiCheckThread = std::thread(&WifiManager::wifiCheckTask, this);
#endif // DEBUG_MODE

    ESP_LOGI(TAG, "WifiManager successfully initialized.");
}

void WifiManager::reconnectTask() {
    ESP_LOGI(TAG, "reconnectTask Started");

    int backoffDelay = 10; // Start with 10 seconds
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return getState() == wifiState_t::DISCONNECTED; });

        ESP_LOGI(TAG, "Attempting to reconnect to Wi-Fi...");

        if (WifiManager::wifi_reconnect_retry_count < WIFI_RECONNECT_ATTEMPT_MAXIMUM_RETRY) {
            ESP_LOGD(TAG, "Retry to connect to the AP");
            ESP_ERROR_CHECK(esp_wifi_connect());
            wifi_reconnect_retry_count++;
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGD(TAG, "Max reconnect attempt exceeded, will restart");
            esp_restart();
        }

        cv.wait_for(lock, std::chrono::seconds(backoffDelay), [this] { return getState() == wifiState_t::CONNECTED; });

        if (isConnected()) {
            ESP_LOGD(TAG, "Connection restored. ReconnectTask will now wait for the next disconnect event.");

            backoffDelay = 10; // Reset backoff delay
        } else {
            ESP_LOGD(TAG, "Failed to reconnect, will retry later...");
            backoffDelay = std::min(backoffDelay * 2, 300); // Exponential backoff, max 300 seconds
        }
    }
}

bool WifiManager::checkInternetConnectivity() {
    // Ping a reliable server (e.g., Google DNS)
    esp_ping_config_t pingConfig = ESP_PING_DEFAULT_CONFIG();
    pingConfig.target_addr.u_addr.ip4.addr = ipaddr_addr("8.8.8.8");
    pingConfig.count = 3; // Send 3 pings

    esp_ping_handle_t pingHandle;
    esp_ping_new_session(&pingConfig, nullptr, &pingHandle);
    esp_ping_start(pingHandle);

    uint32_t receivedCount = 0;
    esp_ping_get_profile(pingHandle, ESP_PING_PROF_REPLY, &receivedCount, sizeof(receivedCount));

    esp_ping_stop(pingHandle);
    esp_ping_delete_session(pingHandle);

    return receivedCount > 0;
}

void WifiManager::checkInternetConnectivityTask() {
    ESP_LOGD(TAG, "checkInternetConnectivityTask Started");

    while (true) {
        std::unique_lock<std::mutex> lock(mtx);

        // Wait for a notification (e.g., WiFi connected event)
        cv.wait_for(lock, std::chrono::seconds(300), [this] { return isConnected(); });

        if (!checkInternetConnectivity()) {
            ESP_LOGD(TAG, "No internet connectivity, triggering reconnection...");
            reconnect();
        }
    }
}

void WifiManager::checkWifiSignalStrength() {
    wifi_ap_record_t ap_info;
    esp_err_t err = esp_wifi_sta_get_ap_info(&ap_info);

    if (err == ESP_OK) {
        ESP_LOGD(TAG, "SSID: %s", ap_info.ssid);
        ESP_LOGD(TAG, "RSSI: %d dBm", ap_info.rssi);
        ESP_LOGD(TAG, "Channel: %d", ap_info.primary);
        ESP_LOGD(TAG, "Authmode: %d", ap_info.authmode);
    } else {
        ESP_LOGE(TAG, "Failed to get AP info: %s", esp_err_to_name(err));
    }
}

void WifiManager::updateCredentials(const std::string& ssid, const std::string& password) {
    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.sta.ssid, ssid.c_str(), sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, password.c_str(), sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_LOGI(TAG, "WiFi credentials updated. Reconnecting...");

    setState(wifiState_t::DISCONNECTED);
    cv.notify_all();
}

void WifiManager::reconnect() {
    setState(wifiState_t::DISCONNECTED);
    cv.notify_all();
}

void WifiManager::eventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    auto& self = WifiManager::getInstance();

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        self.setState(wifiState_t::CONNECTING);
        ESP_ERROR_CHECK(esp_wifi_connect());

    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Wifi connect to the AP FAIL or DISCONNECTED");
        self.setState(wifiState_t::DISCONNECTED);

        // Notify the reconnectTask to start
        self.cv.notify_all();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        auto* event = static_cast<ip_event_got_ip_t*>(event_data);
        ESP_LOGI(TAG, "Wifi Successfully CONNECTED. Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        self.setState(wifiState_t::CONNECTED);
        wifi_reconnect_retry_count = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);

        // Notify the reconnectTask that the connection is restored
        self.cv.notify_all();
    }
}
