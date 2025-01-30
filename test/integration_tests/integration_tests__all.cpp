#include <gtest/gtest.h>

/**
 * Scenarios:
 * - SensorManger: should_aggregate_data_when_multiple_sensors_are_active_given_sensor_manager_is_running
 * - should_publish_data_when_mqtt_is_connected_given_sensor_data_is_available
 * - should_store_data_when_mqtt_fails_given_redundancy_data_storage_is_available
 * - should_log_state_changes_when_sensor_state_changes_given_state_event_logger_is_active
 */

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}