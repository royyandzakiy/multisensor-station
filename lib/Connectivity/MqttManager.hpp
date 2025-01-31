#pragma once
// #include <string>
// #include <atomic>
// #include <thread>
// #include <mutex>
// #include <condition_variable>
// #include <mqtt_client.h>
// #include <esp_log.h>

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <string>
#include <esp_system.h>
#include <nvs_flash.h>
#include <esp_event.h>
#include <esp_netif.h>
// #include <protocol_examples_common.h>
#include <esp_log.h>
#include <mqtt_client.h>

#include <credentials.h>

constexpr const char* MQTT_TAG = "MqttManager";

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
        esp_mqtt_client_config_t mqtt_cfg = {};
        mqtt_cfg.host = CONFIG_MQTT_BROKER_SERVER;
        mqtt_cfg.username = CONFIG_MQTT_BROKER_USERNAME;
        mqtt_cfg.password = CONFIG_MQTT_BROKER_PASSWORD;
        mqtt_cfg.port = 1883;
        // strncpy((char*)mqtt_cfg.uri, CONFIG_MQTT_BROKER_SERVER, sizeof(mqtt_cfg.uri));

        client = esp_mqtt_client_init(&mqtt_cfg);
        esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, &MqttManager::mqtt_event_handler, this);
        esp_mqtt_client_start(client);

        ESP_LOGI(MQTT_TAG, "MqttManager.init end");

        // mqttThread = std::thread(&MqttManager::mqttTask, this);
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

    static void log_error_if_nonzero(const std::string& message, int error_code) {
        if (error_code != 0) {
            ESP_LOGE(MQTT_TAG, "Last error %s: 0x%x", message.c_str(), error_code);
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

    static void mqtt_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data) {
        // auto* self = static_cast<MqttManager*>(handler_args);
        auto& self = MqttManager::getInstance();
        esp_mqtt_event_handle_t event = static_cast<esp_mqtt_event_handle_t>(event_data);
        esp_mqtt_client_handle_t client = event->client;

        ESP_LOGD(MQTT_TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);

        switch (static_cast<esp_mqtt_event_id_t>(event_id)) {
            case MQTT_EVENT_CONNECTED:
                ESP_LOGI(MQTT_TAG, "MQTT_EVENT_CONNECTED");
                self.publish_message(client, "/topic/qos1", "data_3", 1);
                self.subscribe_to_topic(client, "/topic/qos0", 0);
                self.subscribe_to_topic(client, "/topic/qos1", 1);
                self.unsubscribe_from_topic(client, "/topic/qos1");
                break;

            case MQTT_EVENT_DISCONNECTED:
                ESP_LOGI(MQTT_TAG, "MQTT_EVENT_DISCONNECTED");
                break;

            case MQTT_EVENT_SUBSCRIBED:
                ESP_LOGI(MQTT_TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
                self.publish_message(client, "/topic/qos0", "data", 0);
                break;

            case MQTT_EVENT_UNSUBSCRIBED:
                ESP_LOGI(MQTT_TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
                break;

            case MQTT_EVENT_PUBLISHED:
                ESP_LOGI(MQTT_TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
                break;

            case MQTT_EVENT_DATA:
                ESP_LOGI(MQTT_TAG, "MQTT_EVENT_DATA");
                printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
                printf("DATA=%.*s\r\n", event->data_len, event->data);
                break;

            case MQTT_EVENT_ERROR:
                ESP_LOGI(MQTT_TAG, "MQTT_EVENT_ERROR");
                if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                    log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
                    log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
                    log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
                    ESP_LOGI(MQTT_TAG, "Last errno string (%s)", std::strerror(event->error_handle->esp_transport_sock_errno));
                }
                break;

            default:
                ESP_LOGI(MQTT_TAG, "Other event id:%d", event->event_id);
                break;
        }
    }

    void publish_message(esp_mqtt_client_handle_t client, const std::string& topic, const std::string& data, int qos) {
        int msg_id = esp_mqtt_client_publish(client, topic.c_str(), data.c_str(), 0, qos, 0);
        ESP_LOGI(MQTT_TAG, "Sent publish successful, msg_id=%d", msg_id);
    }

    void subscribe_to_topic(esp_mqtt_client_handle_t client, const std::string& topic, int qos) {
        int msg_id = esp_mqtt_client_subscribe(client, topic.c_str(), qos);
        ESP_LOGI(MQTT_TAG, "Sent subscribe successful, msg_id=%d", msg_id);
    }

    void unsubscribe_from_topic(esp_mqtt_client_handle_t client, const std::string& topic) {
        int msg_id = esp_mqtt_client_unsubscribe(client, topic.c_str());
        ESP_LOGI(MQTT_TAG, "Sent unsubscribe successful, msg_id=%d", msg_id);
    }

    esp_mqtt_client_handle_t client{nullptr};
};