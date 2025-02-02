// tests/wifi_manager_test.cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "WifiManager.hpp"
#include "esp_wifi_mock.hpp"

using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;

class WifiManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        espMock = new NiceMock<EspMock>();
    }

    void TearDown() override {
        delete espMock;
        espMock = nullptr;
    }
};

TEST_F(WifiManagerTest, InitTest) {
    EXPECT_CALL(*espMock, esp_wifi_init(_)).WillOnce(Return(ESP_OK));
    EXPECT_CALL(*espMock, esp_wifi_set_mode(WIFI_MODE_STA)).WillOnce(Return(ESP_OK));
    EXPECT_CALL(*espMock, esp_wifi_set_config(WIFI_IF_STA, _)).WillOnce(Return(ESP_OK));
    EXPECT_CALL(*espMock, esp_wifi_start()).WillOnce(Return(ESP_OK));
    EXPECT_CALL(*espMock, esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, _, nullptr, _)).WillOnce(Return(ESP_OK));
    EXPECT_CALL(*espMock, esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, _, nullptr, _)).WillOnce(Return(ESP_OK));

    WifiManager& wifiManager = WifiManager::getInstance();
    wifiManager.init();

    ASSERT_EQ(wifiManager.getState(), wifiState_t::CONNECTING);
}

TEST_F(WifiManagerTest, UpdateCredentialsTest) {
    EXPECT_CALL(*espMock, esp_wifi_set_config(WIFI_IF_STA, _)).WillOnce(Return(ESP_OK));

    WifiManager& wifiManager = WifiManager::getInstance();
    wifiManager.updateCredentials("new_ssid", "new_password");

    ASSERT_EQ(wifiManager.getState(), wifiState_t::DISCONNECTED);
}

TEST_F(WifiManagerTest, IsConnectedTest) {
    WifiManager& wifiManager = WifiManager::getInstance();
    wifiManager.setState(wifiState_t::CONNECTED);

    ASSERT_TRUE(wifiManager.isConnected());
}

// TEST_F(WifiManagerTest, ReconnectTest) {
//     WifiManager& wifiManager = WifiManager::getInstance();
//     wifiManager.setState(wifiState_t::DISCONNECTED);

//     EXPECT_CALL(*espMock, esp_wifi_connect()).WillOnce(Return(ESP_OK));

//     wifiManager.reconnect();

//     ASSERT_EQ(wifiManager.getState(), wifiState_t::DISCONNECTED);
// }

// TEST_F(WifiManagerTest, CheckInternetConnectivityTest) {
//     EXPECT_CALL(*espMock, esp_ping_new_session(_, _, _)).WillOnce(Return(ESP_OK));
//     EXPECT_CALL(*espMock, esp_ping_start(_)).WillOnce(Return(ESP_OK));
//     EXPECT_CALL(*espMock, esp_ping_stop(_)).WillOnce(Return(ESP_OK));
//     EXPECT_CALL(*espMock, esp_ping_delete_session(_)).WillOnce(Return(ESP_OK));
//     EXPECT_CALL(*espMock, esp_ping_get_profile(_, ESP_PING_PROF_REPLY, _, _)).WillOnce(Return(ESP_OK));

//     WifiManager& wifiManager = WifiManager::getInstance();
//     bool result = wifiManager.checkInternetConnectivity();

//     ASSERT_TRUE(result);
// }

// TEST_F(WifiManagerTest, CheckWifiSignalStrengthTest) {
//     wifi_ap_record_t ap_info = {};
//     ap_info.rssi = -50;

//     EXPECT_CALL(*espMock, esp_wifi_sta_get_ap_info(_)).WillOnce(Return(ESP_OK));

//     WifiManager& wifiManager = WifiManager::getInstance();
//     wifiManager.checkWifiSignalStrength();

//     ASSERT_EQ(wifiManager.getState(), wifiState_t::CONNECTED);
// }