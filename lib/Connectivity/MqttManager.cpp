#include <MqttManager.hpp>

void MqttManager::init() {
    esp_mqtt_client_config_t mqtt_cfg = {};
    mqtt_cfg.host = MQTT_SERVER;
    mqtt_cfg.username = MQTT_USERNAME;
    mqtt_cfg.password = MQTT_PASSWORD;
    mqtt_cfg.port = 1883;

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, &MqttManager::eventHandler, this);
    esp_mqtt_client_start(client);
}

int MqttManager::publish(const std::string& topic, const std::string& data) {
    int msg_id;
    if (!isConnected()) {
        ESP_LOGI(TAG, "MQTT not connected, fail to publish message");
        msg_id = -1;
    }
    
    msg_id = esp_mqtt_client_publish(client, topic.c_str(), data.c_str(), 0, 1, 0);
    if (msg_id == -1) {
        ESP_LOGI(TAG, "Failed to publish message, msg_id=%d", msg_id);
    }

    ESP_LOGI(TAG, "Sent publish successful, msg_id=%d", msg_id);
    return msg_id;
}

int MqttManager::subscribe(const std::string& topic) {
    int msg_id = esp_mqtt_client_subscribe(client, topic.c_str(), 1);
    ESP_LOGI(TAG, "Sent subscribe successful, msg_id=%d", msg_id);
    return msg_id;
}

int MqttManager::unsubscribe(const std::string& topic) {
    int msg_id = esp_mqtt_client_unsubscribe(client, topic.c_str());
    ESP_LOGI(TAG, "Sent unsubscribe successful, msg_id=%d", msg_id);
    return msg_id;
}

void MqttManager::eventHandler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data) {
    auto& self = MqttManager::getInstance();
    esp_mqtt_event_handle_t event = static_cast<esp_mqtt_event_handle_t>(event_data);

    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);

    switch (static_cast<esp_mqtt_event_id_t>(event_id)) {
        case MQTT_EVENT_CONNECTED:
            self.setState(mqttState_t::CONNECTED);
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            
            {
                // test
                self.publish("/topic/qos1", "data_3");
                self.subscribe("/topic/qos1");
                self.unsubscribe("/topic/qos1");
            }

            break;

        case MQTT_EVENT_DISCONNECTED:
            self.setState(mqttState_t::DISCONNECTED);
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            
            {
                // test
                self.publish("/topic/qos1", "data");
            }

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