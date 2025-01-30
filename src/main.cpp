#include <WifiManager.hpp>
#include <MqttManager.hpp>
#include <EventLogger.hpp>
#include <LedManager.hpp>
#include <OtaManager.hpp>
#include <SDCardFilesystem.hpp>
#include <SensorManager.hpp>
#include <Sensor.hpp>
#include <SDCardFileSystem.hpp>
#include <memory>

int main() {
    // Initialize Wi-Fi
    WifiManager& wifiManager = WifiManager::getInstance();
    wifiManager.init();
    if (!wifiManager.reconnect()) {
        StateEventLogger::getInstance().logStateChange("WiFi", "Failed to connect");
        LedManager::getInstance().setLEDMode(LEDColor::RED, LEDMode::BLINKING, 500);
        return -1; // Exit if Wi-Fi fails to connect
    }

    // Initialize MQTT
    MqttManager& mqttManager = MqttManager::getInstance();
    mqttManager.init();
    if (!mqttManager.reconnect()) {
        StateEventLogger::getInstance().logStateChange("MQTT", "Failed to connect");
        LedManager::getInstance().setLEDMode(LEDColor::RED, LEDMode::BLINKING, 500);
        return -1; // Exit if MQTT fails to connect
    }

    // Ensure Event Logger and LED Manager are running
    StateEventLogger::getInstance(); // Singleton initialization
    LedManager::getInstance().init();
    LedManager::getInstance().setLEDMode(LEDColor::GREEN, LEDMode::STEADY, 0); // Indicate system is running

    // Check for OTA updates
    OtaManager& otaManager = OtaManager::getInstance();
    otaManager.start();
    if (otaManager.getState() == OtaState::checking || otaManager.getState() == OtaState::downloading) {
        StateEventLogger::getInstance().logStateChange("OTA", "Checking for updates");
        while (otaManager.getState() != OtaState::idle && otaManager.getState() != OtaState::success) {
            // Wait for OTA process to complete
        }
        if (otaManager.getState() == OtaState::success) {
            StateEventLogger::getInstance().logStateChange("OTA", "Update successful, restarting");
            return 0; // Restart the device after OTA update
        }
    }

    // Check SD Card
    SDCardFilesystem& sdCard = SDCardFilesystem::getInstance();
    if (!sdCard.mount()) {
        StateEventLogger::getInstance().logStateChange("SDCard", "Failed to mount");
        LedManager::getInstance().setLEDMode(LEDColor::YELLOW, LEDMode::BLINKING, 500); // Warning: SD card issue
    } else {
        StateEventLogger::getInstance().logStateChange("SDCard", "Mounted successfully");
    }

    // Initialize SensorManager and activate sensors
    std::shared_ptr<Sensor> sensorList[] = {
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
        StateEventLogger::getInstance().logStateChange("System", "Running");

        // Sleep for a short interval to avoid busy-waiting
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}

extern "C" void app_main() {
    main();
}