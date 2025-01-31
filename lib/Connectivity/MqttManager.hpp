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

#include <credentials.h>

enum class mqttState_t {
    NOT_INITIALIZED,
    DISCONNECTED,
    CONNECTED
};

class MqttManager : public StatefulObject<mqttState_t> {
public:
    enum class mqttPublishResult_t {
        SUCCESS,
        ERROR_FAILED,
        ERROR_NOT_CONNECTED
    };

    static MqttManager& getInstance() {
        static MqttManager instance; // Get the singleton instance
        return instance;
    }

    void init() {
        esp_mqtt_client_config_t mqtt_cfg = {};
        mqtt_cfg.host = CONFIG_MQTT_BROKER_SERVER;
        mqtt_cfg.username = CONFIG_MQTT_BROKER_USERNAME;
        mqtt_cfg.password = CONFIG_MQTT_BROKER_PASSWORD;
        mqtt_cfg.port = 1883;

        client = esp_mqtt_client_init(&mqtt_cfg);
        esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, &MqttManager::mqtt_event_handler, this);
        esp_mqtt_client_start(client);
    }

    mqttPublishResult_t publish(const std::string& topic, const std::string& message) {
        if (!isConnected()) {
            return mqttPublishResult_t::ERROR_NOT_CONNECTED;
        }

        int msg_id = esp_mqtt_client_publish(client, topic.c_str(), message.c_str(), 0, 1, 0);
        if (msg_id == -1) {
            return mqttPublishResult_t::ERROR_FAILED;
        }
        return mqttPublishResult_t::SUCCESS;
    }

    bool isConnected() {
        return getState() == mqttState_t::CONNECTED;
    }

private:
    MqttManager()
        : StatefulObject<mqttState_t>("MqttManager", mqttState_t::NOT_INITIALIZED) {
    }

    ~MqttManager() {
    }

    static void log_error_if_nonzero(const std::string& message, int error_code) {
        if (error_code != 0) {
            ESP_LOGE(TAG, "Last error %s: 0x%x", message.c_str(), error_code);
        }
    }

    static void mqtt_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data) {
        auto& self = MqttManager::getInstance();
        esp_mqtt_event_handle_t event = static_cast<esp_mqtt_event_handle_t>(event_data);
        esp_mqtt_client_handle_t client = event->client;

        ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);

        switch (static_cast<esp_mqtt_event_id_t>(event_id)) {
            case MQTT_EVENT_CONNECTED:
                self.setState(mqttState_t::CONNECTED);
                ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
                self.publish_message(client, "/topic/qos1", "data_3", 1);
                self.subscribe_to_topic(client, "/topic/qos0", 0);
                self.subscribe_to_topic(client, "/topic/qos1", 1);
                self.unsubscribe_from_topic(client, "/topic/qos1");
                break;

            case MQTT_EVENT_DISCONNECTED:
                self.setState(mqttState_t::DISCONNECTED);
                ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
                break;

            case MQTT_EVENT_SUBSCRIBED:
                ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
                self.publish_message(client, "/topic/qos0", "data", 0);
                break;

            case MQTT_EVENT_UNSUBSCRIBED:
                ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
                break;

            case MQTT_EVENT_PUBLISHED:
                ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
                break;

            case MQTT_EVENT_DATA:
                ESP_LOGI(TAG, "MQTT_EVENT_DATA");
                printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
                printf("DATA=%.*s\r\n", event->data_len, event->data);
                break;

            case MQTT_EVENT_ERROR:
                ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
                if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                    log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
                    log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
                    log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
                    ESP_LOGI(TAG, "Last errno string (%s)", std::strerror(event->error_handle->esp_transport_sock_errno));
                }
                break;

            default:
                ESP_LOGI(TAG, "Other event id:%d", event->event_id);
                break;
        }
    }

    void publish_message(esp_mqtt_client_handle_t client, const std::string& topic, const std::string& data, int qos) {
        int msg_id = esp_mqtt_client_publish(client, topic.c_str(), data.c_str(), 0, qos, 0);
        ESP_LOGI(TAG, "Sent publish successful, msg_id=%d", msg_id);
    }

    void subscribe_to_topic(esp_mqtt_client_handle_t client, const std::string& topic, int qos) {
        int msg_id = esp_mqtt_client_subscribe(client, topic.c_str(), qos);
        ESP_LOGI(TAG, "Sent subscribe successful, msg_id=%d", msg_id);
    }

    void unsubscribe_from_topic(esp_mqtt_client_handle_t client, const std::string& topic) {
        int msg_id = esp_mqtt_client_unsubscribe(client, topic.c_str());
        ESP_LOGI(TAG, "Sent unsubscribe successful, msg_id=%d", msg_id);
    }

    std::mutex mtx;
    std::condition_variable cv;

    static constexpr const char* TAG = "MqttManager";

    esp_mqtt_client_handle_t client{nullptr};
};