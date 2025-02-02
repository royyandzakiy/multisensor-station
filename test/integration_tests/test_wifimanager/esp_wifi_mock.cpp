// mocks/esp_wifi_mock.cpp
#include "esp_wifi_mock.hpp"

EspMock* espMock = nullptr;

extern "C" {
    int esp_wifi_init(const wifi_init_config_t* config) {
        return espMock->esp_wifi_init(config);
    }

    int esp_wifi_set_mode(wifi_mode_t mode) {
        return espMock->esp_wifi_set_mode(mode);
    }

    int esp_wifi_set_config(wifi_interface_t interface, wifi_config_t* conf) {
        return espMock->esp_wifi_set_config(interface, conf);
    }

    int esp_wifi_start() {
        return espMock->esp_wifi_start();
    }

    int esp_wifi_connect() {
        return espMock->esp_wifi_connect();
    }

    int esp_wifi_sta_get_ap_info(wifi_ap_record_t* ap_info) {
        return espMock->esp_wifi_sta_get_ap_info(ap_info);
    }

    int esp_event_handler_instance_register(esp_event_base_t event_base, int32_t event_id, esp_event_handler_t event_handler, void* event_handler_arg, esp_event_handler_instance_t* instance) {
        return espMock->esp_event_handler_instance_register(event_base, event_id, event_handler, event_handler_arg, instance);
    }

    // int esp_ping_new_session(const esp_ping_config_t* config, const esp_ping_callbacks_t* cbs, esp_ping_handle_t* hdl) {
    //     return espMock->esp_ping_new_session(config, cbs, hdl);
    // }

    // int esp_ping_start(esp_ping_handle_t hdl) {
    //     return espMock->esp_ping_start(hdl);
    // }

    // int esp_ping_stop(esp_ping_handle_t hdl) {
    //     return espMock->esp_ping_stop(hdl);
    // }

    // int esp_ping_delete_session(esp_ping_handle_t hdl) {
    //     return espMock->esp_ping_delete_session(hdl);
    // }

    // int esp_ping_get_profile(esp_ping_handle_t hdl, esp_ping_profile_t profile, void* data, uint32_t size) {
    //     return espMock->esp_ping_get_profile(hdl, profile, data, size);
    // }
}