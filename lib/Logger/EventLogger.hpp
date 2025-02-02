#pragma once
#include <functional>
#include <vector>
#include <string>
#include <mutex>
#include <esp_log.h>

#define USE_CIRCULAR_BUFFER
// #define USE_VECTOR

#ifdef USE_CIRCULAR_BUFFER
#define CIRCULAR_BUFFER_MAX_SIZE 30
#include <CircularBuffer.hpp>
#elif defined(USE_VECTOR)
// include nothing
#endif

class EventLogger {
public:
    using LogCallback = std::function<void(const std::string& id, const std::string& state)>;

    static EventLogger& getInstance() {
        static EventLogger instance; // Get the singleton instance
        return instance;
    }
       
    void logStateChange(const std::string& id, const std::string& state) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            static char buffer[64];
            snprintf(buffer, sizeof(buffer), "[%s] %s -> %s", getCurrentTimestamp().c_str(), id.c_str(), state.c_str());
            ESP_LOGI(TAG, "%s", buffer);
#ifdef USE_CIRCULAR_BUFFER
            logs.push(std::string(buffer));
        } // make mutex go out of bounds

        if (logs.is_max()) {
            printLatestLogs();
        }
#elif defined(USE_VECTOR)
            logs.push_back(std::string(buffer));
#endif

        // // Notify observers, still error
        // for (const auto& callback : callbacks) {
        //     for (int i=0; i<callbacks.size(); i++) {
        //         auto& callback = callbacks[i];
        //         callback(id, state);
        //     }
        // }
    }

    void registerCallback(const LogCallback& callback) {
        std::lock_guard<std::mutex> lock(mtx);
        callbacks.fill(callback);
    }

    void printLatestLogs() {
        std::lock_guard<std::mutex> lock(mtx);
        for (std::size_t i = 0; i < logs.size(); ++i) {
            const std::optional<std::string>& log = logs.at(i);
            if (log.has_value()) {
                ESP_LOGI(TAG, "%s", log.value().c_str());
            }
        }
    }

private:
    EventLogger() = default; // Private constructor for Singleton
    ~EventLogger() {}

    EventLogger(const EventLogger&) = delete;
    EventLogger& operator=(const EventLogger&) = delete;

    std::string getCurrentTimestamp() const {
        // not yet implemented
        // auto now = std::chrono::system_clock::now();
        // auto in_time_t = std::chrono::system_clock::to_time_t(now);
        // std::stringstream ss;
        // ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
        // return ss.str();
        std::string timestamp = "<timestamp_not_implemented>";
        return timestamp;
    }

#ifdef USE_CIRCULAR_BUFFER
    CircularBuffer<std::string, CIRCULAR_BUFFER_MAX_SIZE> logs;
#elif defined(USE_VECTOR)
    std::vector<std::string> logs;               // In-memory log storage
#endif
    std::array<LogCallback, 10> callbacks;          // Observer callbacks
    mutable std::mutex mtx;                              // Mutex for thread safety
    static constexpr size_t logInterval = 10;    // Store logs every 10 entries
    static constexpr const char* TAG = "EventLogger";
};