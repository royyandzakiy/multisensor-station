# Multi Sensor Box

## Getting Started
- 

## Development

## Design
- Sensor<T>: Observable
    - sensorInit(), peekSensorDataCircularBufferQueue(), activateSensorDataAcquisitionTask(), T sensorDataType()
    - sensorDataAcquisitionTask(), T sensorRead(), notifyObservers()
    - CircularBufferQueue<T> sensorDataQueue, string sensorName, observerList<Observer>[]
    - design thoughts:
        - sensorRead() returns the expected data type, it hides any details like sample rate, sample window size, FFT algorithm, etc
            - this class is observable because it can have different runtime flows
        - sensorDataAcquisitionTask() runs sensorRead() within an expected interval, making sure sensorDataQueue gets filled 
            - it also calls to notifyObservers() and let them peek the latest data
- CircularBufferQueue<T>
    - enqueue(T), T dequeue(), int size()
    - queue<T>[]
- SensorController<Sensor>
    - SensorController(Sensor * sensorList[]), activateAllSensorDataAcquisitionTasks()
    - sensorList<Sensor>[]
    - design thoughts:
        - sensors are expected to never stop running data acquisition task throughout the firmware lifetime
- enum SensorInitResult
    - success, failure, timeout, device_not_found, unknown_error

- SensorDataPublisher: observer
    - notifiedBySensor(sensor&), prepareSensorDataForPublishing(T sensorData&), publishProcessedData(PublishableSensorData p&)
    - RedundancyDataStorage rds
    - design thoughts:
        - prepareSensorDataForPublishing() checks for sensorDataType(), then prepares accordingly (making it into json string)
        - publishProcessedData() publishes an expected string format, if successful, will call for dequeue of data, else, store in redundancyDataStorage
- RedundancyDataStorage
    - store(PublishableSensorData p&), publishStoredDataTask(), string getLatestData(), removeLatestData()
    - design thoughts:
        - this doesn't know nor care about different sensors, it just sees a big blob of stored strings
        - the publish task simply gets and removes data when successfully published. it gets activated from the beginning, yet gets a very low priority
- json PublishableSensorData
    - timestamp, string data, string sensorName

- StateMachine<State>
    - changeTo(State *)
    - stateChangeList<State>[]
    - currentState
    - design thoughts:
        - it should orchestrate between sensor data acquisition task activate, sensor read, sensor data publish. but no, because sensor data publish is a seperate entity, the loose couple is the main point. sensor read should never wait for the sensor publish to succeed
        - tbh, I'm still not quite sure which class should implement this, and why
- State<Sensor>
    - run(Sensor *)


### Requirements
- Create system with multiple sensors, these sensors are not yet fixed so make it so that we can swap in new sensors every now and then
- One sensor should never be a single point of failure, make it easy to identify which one is problematic, and keep the other working reliably anytime
- Most valuable thing is the data transmission, make sure this never get's disrupted by other working components. Capture most important system parts using a Finite State Machine (FSM) and Error enum based logging, to help later debugging
- Connectivty is by WiFi, it should be always active, but always mitigate if there are failures. If so, keep data for around 5 days, and retransmit the data with each correct timestamps
- System is monitorable and updatable (OTA) through fleet management cloud service
- Create system with proper testing suites
- Implement using latest C++20