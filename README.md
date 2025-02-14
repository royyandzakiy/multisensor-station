# Multi Sensor Box
This repository contains a robust and modular (mostly) C++20-based system designed for acquiring, managing, and transmitting sensor data in a reliable and fault-tolerant manner. The system is built with flexibility in mind, allowing for easy integration of various sensors and ensuring that sensor data is never lost, even in the event of connectivity issues or sensor failures.

## Key Features
- **Modular Sensor Integration**: The system supports multiple sensors with a design that allows for easy swapping and addition of new sensors. Each sensor operates independently, ensuring that a failure in one sensor does not affect others.
- **Data Redundancy and Reliability**: Sensor data is stored in a circular buffer queue and backed up in a redundancy storage system. If connectivity is lost, data is retained with timestamps and retransmitted once connectivity is restored.
- **State Management and Logging**: The system uses a StatefulObject pattern to manage the state of various components (e.g., sensors, WiFi, MQTT) and an EventLogger to capture state changes and important events for debugging purposes.
- **Connectivity**: The system maintains a persistent WiFi connection and uses MQTT for data transmission. It includes mechanisms to handle connectivity failures and ensure data is transmitted reliably.
- **Over-the-Air (OTA) Updates**: The system supports OTA updates, allowing for remote monitoring and updates through a fleet management cloud service.
- **Testing and Separation of Concerns**: The system is designed with a clear separation of concerns, making it easy to test individual components. Native C++20 features are used to ensure modern and efficient code.

## Getting Started
### Prerequisites
- VS Code with PlatformIO IDE installed (PlatformIO core version `6.1.16`, framework-espidf @ `3.50400.0` (ESP IDF version `5.4.0`))
- ESP32 connected via USB.
### Steps
- Clone the Repository:
    ```bash
    git clone <repository-url>
    cd <repository-folder>
    ```
- Open in VS Code
- Change `src/credentials.h.change_this` to just `credentials.h`
- Build & Upload

---

# Developer Zone
## Development Notes
- Requirements & Architecture
- Phase 1: Basic Functions
    - [x] Patterns
    - [x] Connectivity
        - connect wifi & mqtt
        - reconnect wifi
        - publish to mqtt
    - [x] Partitions
        - reduce image size to be able for upload
    - [x] Logger
        - successfully logging state change of all StatefulObjectLogged
    - [x] Circular Buffer
        - success implementing logic & test cases
    - [ ] OTA, thingsboard, https cert
    - [ ] task profiler, tasks list
- Phase 2: Sensing
    - [ ] Sensor (dummy), SensorManager
    - [ ] Sampler
    - [ ] Core Dump
    - [ ] Recipe/Pipeline
    - [ ] Time, RTC
    - [ ] DataAcquisitor
    - [ ] Filesystem (spiffs)
- Phase 3
    - [ ] Sensor (real) -- CAN, Mod, SPI, I2C, UART, Signal
    - [ ] Filesystem (sdcard)
    - [ ] Config, Config Manager, mutex
    - [ ] Device Identity, Device Provisioning
    - [ ] Device heartbeat
- Phase 4
    - [ ] gsm manager
    - [ ] internet manager
    - [ ] Config Menu
    - [ ] ble interface
    - [ ] wifi secure, jwt
    - [ ] web async interface, mdns
    - [ ] button interface
    - [ ] LED, Beeper
- Phase 5
    - [ ] control motor (to rotate)
    - [ ] sense motor movement (encoder)

## Design
```
+-------------------+       +-------------------+       +----------------------+
|     Sensor        |       |  SensorManager    |       |  SensorDataPublisher |
|  (Observable)     |<------|  (Manages Sensors)|<------|  (Observer)          |
+-------------------+       +-------------------+       +----------------------+
        |                                                       |
        v                                                       v
+-------------------+       +-------------------+       +-------------------+
| CircularBuffer    |       |  RedundancyData   |       |  MqttManager      |
|  Queue (Storage)  |------>|  Storage (Backup) |<------|  (Publishes Data) |
+-------------------+       +-------------------+       +-------------------+
                                    |                           |
        +---------------------------+                           |
        v                                                       v
+-------------------+       +-------------------+       +-------------------+
|  FileSystem       |       |  EventLogger      |       |  WifiManager      |
|  (SD Card)        |<------|  (Logs Events)    |<------|  (Handles WiFi)   |
+-------------------+       +-------------------+       +-------------------+
        |                           ^                           |
        v                           |                           v
+-------------------+       +-------------------+       +-------------------+
|  OtaManager       |       |  LedManager       |       |  Config           |
|  (OTA Updates)    |<------|  (LED Indicators) |       |  (WiFi/MQTT/etc)  |
+-------------------+       +-------------------+       +-------------------+
```

```markdown
### Sensor
- General Flow: read then queue & notify > publish else store > dequeue -- it is expected to have these function calls seperate to keep data redundancy
- Sensor<SensorDataType>: Observable, StatefulObject<SensorState>
    + sensorInit(), peekSensorDataCircularBufferQueue(), startSensorDataAcquisitionTask(), T sensorDataType()
    - sensorDataAcquisitionTask(), T sensorRead(), notifyObservers()
    - CircularBufferQueue<SensorDataType> sensorDataQueue, string sensorName, observerList<Observer>[]
    > design thoughts:
        - sensorInit() any peripheral based configuration, like gpio pin, spi, etc
        - sensorRead() returns the expected data type, it hides any details like sample rate, sample window size, FFT algorithm, etc
        - sensor data is accessed via callback (Observable) because sensorRead can have different runtimes flows
        - sensorDataAcquisitionTask() runs sensorRead() within an expected interval, making sure sensorDataQueue gets filled 
            - it also calls to notifyObservers() and let them peek the latest data
- enum SensorInitResult
    + success, failure, timeout, device_not_found, unknown_error
- enum SensorState
    + init, task_running, task_stop
- CircularBufferQueue<T>: StatefulObject<>
    + CircularBufferQueue(maxSize), enqueue(T), T dequeue(), int size(), isFull()
    - queue<T>[], int maxSize
- CircularBufferQueueState
    + empty, has_data, is_full, data_discarded_because_full
- SensorManager<Sensor>
    + SensorManager(Sensor shared_ptr sensorList[]), startAllSensorDataAcquisitionTasks()
    - shared_ptr sensorList<Sensor>[]
    > design thoughts:
        - sensors are by design expected to never stop running data acquisition task throughout the firmware lifetime, therefore doesn't implement any sensor task controls here

### Data Acquisitor
- SensorDataPublisher: Observer
    + notifiedBySensor(sensor&), prepareSensorDataForPublishing(T sensorData&), publishProcessedData(PublishableSensorData p&)
    - RedundancyDataStorage rds, MqttManager_ptr& mqttManager
    - design thoughts:
        - this is a singleton
        - prepareSensorDataForPublishing() checks for sensorDataType(), then prepares accordingly (making it into json string)
        - publishProcessedData() publishes an expected string format, if successful, will call for dequeue of data, else, store in redundancyDataStorage
- RedundancyDataStorage
    + store(PublishableSensorData p&), publishStoredDataTask(), string getLatestData(), removeLatestData()
    - FileSystemManager fs
    > design thoughts:
        - this doesn't know nor care about different sensors, it just sees a big blob of stored strings
        - the publish task simply gets and removes data when successfully published. it gets activated from the beginning, yet gets a very low priority
- json PublishableSensorData
    + getJson(), getString()
    - serializeToString()
    - jsonData {timestamp, string data, string sensorName}

### Connectivity
- WifiManager: StatefulObject<WifiState>
    + reconnect(), isConnected()
    - init(), {thread}, stop(), wifiTask(), static wifiEventHandler()
    - wifiThread
- enum WifiState
    + connecting, fail_to_connect, disconnected, connected
- MqttManager: StatefulObject<MqttState>
    + publish(string& topic, string& message), reconnect(), isConnected()
    - init() {setState(), thread}, stop(), mqttTask(), static mqttEventHandler
    - mqttThread
- enum PublishResult
    + success, fail, not_connected
- enum MqttState
    + connected, disconnected, error

### OTA
- OtaManager: StatefulObject<OtaState>
    + start()
    - monitorOtaBtton(), enterOtaMode(), checkForOtaUpdates(), checkOtaAvailability(), performOtaUpdate()
- enum OtaState
    + idle, checking, downloading, applying, success, failed

### Config
- WifiConfig
    - apName, apPassword
- MqttConfig
    - username, password, server
- CircularBufferQueueConfig
    - maxSize

### Logger
- EventLogger
    + EventLogger() {thread}, logStateChange(const string& id, const T& state), printLatestLogs()
    - storeLogs(), std::string getCurrentTimestamp()
    - vector<string> logs, mutex mtx, thread storeLogsTask
    > design thoughts:
        - usecase to help debugging failures, it uses a limited circularbuffer and a fixed log buffer

### File System
- SDCardFilesystem
    + bool mount(), unmount(), bool writeFile(string& path, string& data), string readFile(string& path), bool deleteFile(string& path), bool appendLine(string& path, string& line), string readFirstLine(string& path), bool removeFirstLine(string& path)
    > design thoughts:
        - removeFirstLine() is going to be costly, because every removal requires to rewrite the whole file. this is okay, as it will be called in a low priority task, and (for now) assumed that the sd card size is infinity

### LedManager
- LedManager
    + static & getInstance(), init(), setLEDMode(LEDColor, LEDMode, intervalMs)
    - LedManager() {EventLogger::getInstance().registerLedCallback}, processLog, updateLedState(LEDColor), blinkingTask(), getPinForColor(LEDColor), string getColorName(LEDColor), string getModeName(LEDMode)
    - unordered_map<LEDColor, LEDMode> ledModes, unordered_map<LEDColor, int> ledIntervals, redPin, greenPin, bluePin, thread blinkingThread, mutex mtx, atomic<bool> running
- enum LedColor
- enum LedMode

### Patterns
- Observer
    + notified()
- Observable
    + notifyObservers()
    - Observer observerList[]
- StatefulObject<T>
    + setState(T const p), T getState()
    - string id, T state
```

## Requirements
- Create system that enables multiple sensors, hardware wise, these sensors are not yet fixed so make it so that we can swap in new sensors every now and then
- One sensor should never be a single point of failure, if one fails, the others should keep working reliably anytime; for debugging, make it easy to identify which sensor is problematic 
- Most valuable thing is the sensor data, make sure the sensor data never unintentionally goes missing
- Capture most important parts using a Stateful and EventLogger to later help with debugging
- Connectivty is by WiFi, it should be always active, but always mitigate if there are failures. If so, keep data (with each correct timestamps) for around X time, then flush aka retransmit the unsent data
- System is monitorable and updatable (OTA) through fleet management cloud service
- Create system with proper testing suites. Design such that it has good seperation of concerns and implement native C++20 code as much as possible