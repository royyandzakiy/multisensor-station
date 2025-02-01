#pragma once
#include <functional>
#include <vector>
#include <string>
#include <mutex>
#include <esp_log.h>

// #define CB
#ifdef CB
#include <CircularBuffer.hpp>
#endif

class EventLogger {
public:
    using LogCallback = std::function<void(const std::string& id, const std::string& state)>;

    static EventLogger& getInstance() {
        static EventLogger instance; // Get the singleton instance
        return instance;
    }

    // Log a state change
    void logStateChange(const std::string& id, const std::string& state) {
        std::lock_guard<std::mutex> lock(mtx);
        static char buffer[64];
        snprintf(buffer, sizeof(buffer), "[%s] %s -> %s", getCurrentTimestamp().c_str(), id.c_str(), state.c_str());
        ESP_LOGI(TAG, "%s", buffer);
#ifdef CB
        // logs.push(std::string(buffer));
#else
        logs.push_back(std::string(buffer));
#endif

        // Notify observers
        for (const auto& callback : callbacks) {
            callback(id, state);
        }
    }

    void registerCallback(const LogCallback& callback) {
        std::lock_guard<std::mutex> lock(mtx);
        callbacks.push_back(callback);
    }

    void debug_printLatestLogs() const {
        std::lock_guard<std::mutex> lock(mtx);
        for (const auto& log : logs) {
            ESP_LOGI(TAG, "%s", log.c_str());
        }
    }

private:
    EventLogger() = default; // Private constructor for Singleton
    ~EventLogger() {}

    EventLogger(const EventLogger&) = delete;
    EventLogger& operator=(const EventLogger&) = delete;

    std::string getCurrentTimestamp() const {
        // auto now = std::chrono::system_clock::now();
        // auto in_time_t = std::chrono::system_clock::to_time_t(now);
        // std::stringstream ss;
        // ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
        // return ss.str();
        std::string timestamp = "<timestamp_not_implemented>";
        return timestamp;
    }

#ifdef CB
    CircularBuffer<std::string, 10> logs;
#else
    std::vector<std::string> logs;               // In-memory log storage
#endif
    std::vector<LogCallback> callbacks;          // Observer callbacks
    mutable std::mutex mtx;                              // Mutex for thread safety
    static constexpr size_t logInterval = 10;    // Store logs every 10 entries
    static constexpr const char* TAG = "EventLogger";
};