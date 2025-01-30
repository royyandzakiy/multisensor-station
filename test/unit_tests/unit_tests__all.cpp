#include <gtest/gtest.h>

/**
 * Scenarios:
 * - Sensor: should_return_valid_data_when_sensor_read_is_called_given_sensor_is_initialized
 * - Sensor: should_enqueue_data_when_sensor_data_acquisition_task_is_running_given_sensor_is_initialized
 * - Sensor: should_notify_observers_when_sensor_data_is_available_given_observers_are_registered
 * - Wifi: should_reconnect_when_wifi_disconnects_given_wifi_manager_is_active
 * - SDCardFileSystem: should_store_data_when_write_file_is_called_given_sd_card_is_mounted
 */

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}