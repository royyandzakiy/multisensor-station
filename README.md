# Multi Sensor Box

## Getting Started

## Development Notes
- requirement & architecture
- phase 1
    - patterns
    - connectivity
        - connect wifi & mqtt; publish to mqtt;
    - logger
        - successfully logging state change of all statefulobject (bug of T type);
        - option 1: when init stateful object add param to activate logger
        - option 2: stateful object becomes observable, add logger in the init
    - reduce image size to be able for upload
    - ota
- phase 2
    - sensor (dummy), circularbufferqueue, sensormanager
    - dataacquisitor
    - filesystem (spiffs)
- phase 3
    - sensor (real)
    - filesystem (sdcard)
    - config

## Design
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