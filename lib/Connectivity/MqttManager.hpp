#pragma once
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <mqtt_client.h>
#include <esp_log.h>
#include <credentials.h>

class MqttManager : public StatefulObject<std::string> {
public:
    enum class PublishResult {
        SUCCESS,
        FAILED,
        NOT_CONNECTED
    };

    // Get the singleton instance
    static MqttManager& getInstance() {
        static MqttManager instance;
        return instance;
    }

    void init() {
        esp_mqtt_client_config_t mqtt_cfg = {
            .uri = CONFIG_MQTT_BROKER_URI,
        };

        mqttClient = esp_mqtt_client_init(&mqtt_cfg);
        esp_mqtt_client_register_event(mqttClient, MQTT_EVENT_ANY, &MqttManager::mqttEventHandler, this);
        esp_mqtt_client_start(mqttClient);
        setState("CONNECTING");

        mqttThread = std::thread(&MqttManager::mqttTask, this);
    }

    PublishResult publish(const std::string& topic, const std::string& message) {
        if (getState() != "CONNECTED") {
            return PublishResult::NOT_CONNECTED;
        }

        int msg_id = esp_mqtt_client_publish(mqttClient, topic.c_str(), message.c_str(), 0, 1, 0);
        if (msg_id == -1) {
            return PublishResult::FAILED;
        }
        return PublishResult::SUCCESS;
    }

    bool reconnect() {
        // insert logic here later...
        return true;
    }

    bool isConnected() {
        // insert logic here later...
        return true;
    }

private:
    std::thread mqttThread;
    std::mutex mtx;
    std::condition_variable cv;
    bool running = true;
    esp_mqtt_client_handle_t mqttClient;

    MqttManager()
        : StatefulObject<std::string>("MqttManager", "DISCONNECTED") {
    }

    ~MqttManager() {
        stop();
    }

    void stop() {
        running = false;
        cv.notify_all();
        if (mqttThread.joinable()) {
            mqttThread.join();
        }
    }

    static void mqttEventHandler(void* arg, esp_event_base_t base, int32_t event_id, void* event_data) {
        auto* self = static_cast<MqttManager*>(arg);
        esp_mqtt_event_handle_t event = static_cast<esp_mqtt_event_handle_t>(event_data);

        switch (event->event_id) {
            case MQTT_EVENT_CONNECTED:
                self->setState("CONNECTED");
                break;
            case MQTT_EVENT_DISCONNECTED:
                self->setState("DISCONNECTED");
                break;
            case MQTT_EVENT_ERROR:
                self->setState("ERROR");
                break;
            default:
                break;
        }
    }

    void mqttTask() {
        while (running) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait_for(lock, std::chrono::seconds(1));
            // Periodically check or perform actions here
        }
    }
};