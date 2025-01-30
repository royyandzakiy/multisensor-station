#pragma once
#include <memory>
#include <Sensor.hpp>

class SensorManager {
public:
    SensorManager(std::vector<std::shared_ptr<Sensor>> sensorList): sensorList(sensorList) {}

    void startAllSensorDataAcquisitionTasks() {}

private:
    std::vector<std::shared_ptr<Sensor>> sensorList;
};

class SensorDataPublisher {
public:
    // Get the singleton instance
    static SensorDataPublisher& getInstance() {
        static SensorDataPublisher instance;
        return instance;
    }
};

class RedundancyDataStorage {
public:
    // Get the singleton instance
    static RedundancyDataStorage& getInstance() {
        static RedundancyDataStorage instance;
        return instance;
    }

    void publishStoredDataTask() {}
};