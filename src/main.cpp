#include <esp_log.h>
#include <memory>
#include <nvs_flash.h>

#include <WifiManager.hpp>
#include <MqttManager.hpp>
#include <EventLogger.hpp>
// #include <OtaManager.hpp>
// #include <LedManager.hpp>
// #include <SDCardFilesystem.hpp>
// #include <SensorManager.hpp>
// #include <Sensor.hpp>
// #include <SDCardFileSystem.hpp>

constexpr const char* MAIN_TAG = "MAIN";

int main() {
    ESP_LOGI(MAIN_TAG, "========= Main Start =========");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    WifiManager& wifiManager = WifiManager::getInstance();
    wifiManager.init();
    ESP_LOGI(MAIN_TAG, "========= Main End =========");

    MqttManager& mqttManager = MqttManager::getInstance();
    mqttManager.init();

#ifdef NOT_YET_DEVELOPED

    // Ensure Event Logger and LED Manager are running
    EventLogger::getInstance(); // Singleton initialization
    LedManager::getInstance().init();
    LedManager::getInstance().setLEDMode(LedManager::LEDColor::GREEN, LedManager::LEDMode::ON, 0); // Indicate system is running

    // Check for OTA updates
    OtaManager& otaManager = OtaManager::getInstance();
    otaManager.start();
    if (otaManager.getState() == OtaState::Checking || otaManager.getState() == OtaState::Downloading) {
        EventLogger::getInstance().logStateChange("OTA", "Checking for updates");
        while (otaManager.getState() != OtaState::Idle && otaManager.getState() != OtaState::Success) {
            // Wait for OTA process to complete
        }
        if (otaManager.getState() == OtaState::Success) {
            EventLogger::getInstance().logStateChange("OTA", "Update successful, restarting");
            return 0; // Restart the device after OTA update
        }
    }

    // Check SD Card
    SDCardFilesystem& sdCard = SDCardFilesystem::getInstance();
    if (!sdCard.mount()) {
        EventLogger::getInstance().logStateChange("SDCard", "Failed to mount");
        LedManager::getInstance().setLEDMode(LedManager::LEDColor::RED, LedManager::LEDMode::BLINK, 500); // Warning: SD card issue
    } else {
        EventLogger::getInstance().logStateChange("SDCard", "Mounted successfully");
    }

    // Initialize SensorManager and activate sensors
    std::vector<std::shared_ptr<Sensor>> sensorList = {
        std::make_shared<Sensor>("TemperatureSensor"),
        std::make_shared<Sensor>("HumiditySensor"),
        std::make_shared<Sensor>("PressureSensor")
    };
    SensorManager sensorManager(sensorList);
    sensorManager.startAllSensorDataAcquisitionTasks();

    // Initialize SensorDataPublisher and RedundancyDataStorage
    SensorDataPublisher& sensorDataPublisher = SensorDataPublisher::getInstance();
    RedundancyDataStorage& redundancyDataStorage = RedundancyDataStorage::getInstance();

    // Main loop
    while (true) {
        // Check Wi-Fi and MQTT connectivity
        if (!wifiManager.isConnected()) {
            wifiManager.reconnect();
        }
        if (!mqttManager.isConnected()) {
            mqttManager.reconnect();
        }

        // Publish stored data if any
        redundancyDataStorage.publishStoredDataTask();

        // Log system state periodically
        EventLogger::getInstance().logStateChange("System", "Running");

        // Sleep for a short interval to avoid busy-waiting
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

#endif // NOT_YET_DEVELOPED

    return 0;
}

extern "C" void app_main() {
    main();
}