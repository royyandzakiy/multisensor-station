#include <gtest/gtest.h>

/**
 * Scenarios:
 * - should_send_data_when_all_sensors_are_functional_given_device_is_connected
 * - should_store_data_locally_when_mqtt_fails_given_sensors_are_functional
 * - should_notify_sensor_failure_when_a_sensor_malfunctions_given_sensor_manager_is_running
 * - should_reconnect_when_wifi_disconnects_given_wifi_manager_is_active
 */

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}