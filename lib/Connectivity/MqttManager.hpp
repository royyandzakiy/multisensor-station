#pragma once
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <string>
#include <esp_system.h>
#include <nvs_flash.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <esp_log.h>
#include <mqtt_client.h>
#include <condition_variable>
#include <mutex>

#include <../../src/credentials.h>
#include <StatefulObject.hpp>

/* MQTT configuration */
constexpr auto MQTT_SERVER = CONFIG_MQTT_BROKER_SERVER;
constexpr auto MQTT_USERNAME = CONFIG_MQTT_BROKER_USERNAME;
constexpr auto MQTT_PASSWORD = CONFIG_MQTT_BROKER_PASSWORD;

enum class mqttState_t {
    NOT_INITIALIZED,
    DISCONNECTED,
    CONNECTED
};

class MqttManager : public StatefulObjectLogged<mqttState_t> {
public:
    std::string mqttStateToString(mqttState_t state) const {
        switch (state) {
            case mqttState_t::NOT_INITIALIZED: return "NOT_INITIALIZED";
            case mqttState_t::DISCONNECTED: return "DISCONNECTED";
            case mqttState_t::CONNECTED: return "CONNECTED";
            default: return "UNKNOWN";
        }
    }

    static MqttManager& getInstance() {
        static MqttManager instance; // Get the singleton instance
        return instance;
    }

    void init();

    bool isConnected() {
        return getState() == mqttState_t::CONNECTED;
    }
    
    int publish(const std::string& topic, const std::string& data);

    int subscribe(const std::string& topic);

    int unsubscribe(const std::string& topic);

private:
    MqttManager()
        : StatefulObjectLogged<mqttState_t>("MqttManager", mqttState_t::NOT_INITIALIZED) {
    }

    ~MqttManager() {
    }

    std::string getStateAsString() const override {
        return mqttStateToString(getState()); // Use the custom conversion function
    }

    esp_mqtt_client_handle_t getMqttClient() {
        return client;
    }

    static void log_error_if_nonzero(const std::string& message, int error_code) {
        if (error_code != 0) {
            ESP_LOGE(TAG, "Last error %s: 0x%x", message.c_str(), error_code);
        }
    }

    static void eventHandler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data);

    std::mutex mtx;
    std::condition_variable cv;

    static constexpr const char* TAG = "MqttManager";

    esp_mqtt_client_handle_t client{nullptr};
};