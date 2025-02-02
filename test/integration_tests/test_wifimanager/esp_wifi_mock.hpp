// mocks/esp_mock.hpp
#ifndef ESP_MOCK_HPP
#define ESP_MOCK_HPP

#include <gmock/gmock.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_ping.h>

class EspMock {
public:
    virtual ~EspMock() = default;

    MOCK_METHOD(int, esp_wifi_init, (const wifi_init_config_t* config));
    MOCK_METHOD(int, esp_wifi_set_mode, (wifi_mode_t mode));
    MOCK_METHOD(int, esp_wifi_set_config, (wifi_interface_t interface, wifi_config_t* conf));
    MOCK_METHOD(int, esp_wifi_start, ());
    MOCK_METHOD(int, esp_wifi_connect, ());
    MOCK_METHOD(int, esp_wifi_sta_get_ap_info, (wifi_ap_record_t* ap_info));
    MOCK_METHOD(int, esp_event_handler_instance_register, (esp_event_base_t event_base, int32_t event_id, esp_event_handler_t event_handler, void* event_handler_arg, esp_event_handler_instance_t* instance));
    // MOCK_METHOD(int, esp_ping_new_session, (const esp_ping_config_t* config, const esp_ping_callbacks_t* cbs, esp_ping_handle_t* hdl));
    // MOCK_METHOD(int, esp_ping_start, (esp_ping_handle_t hdl));
    // MOCK_METHOD(int, esp_ping_stop, (esp_ping_handle_t hdl));
    // MOCK_METHOD(int, esp_ping_delete_session, (esp_ping_handle_t hdl));
    // MOCK_METHOD(int, esp_ping_get_profile, (esp_ping_handle_t hdl, esp_ping_profile_t profile, void* data, uint32_t size));
};

extern EspMock* espMock;

#endif // ESP_MOCK_HPP