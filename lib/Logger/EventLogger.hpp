#pragma once
#include <functional>
#include <vector>
#include <string>
#include <mutex>
#include <iostream> // do not use
#include <iomanip> // do not use
#include <sstream> // do not use

class EventLogger {
public:
    using LogCallback = std::function<void(const std::string& id, const std::string& state)>;

    // Get the singleton instance
    static EventLogger& getInstance() {
        static EventLogger instance;
        return instance;
    }

    // Log a state change
    void logStateChange(const std::string& id, const std::string& state) {
        std::lock_guard<std::mutex> lock(mtx);
        std::stringstream ss;
        ss << "[" << getCurrentTimestamp() << "] " << id << " -> " << state;
        logs.push_back(ss.str());

        // Notify observers
        for (const auto& callback : callbacks) {
            callback(id, ss.str());
        }

        // Store logs in intervals (e.g., every 10 logs)
        if (logs.size() >= logInterval) {
            storeLogs();
        }
    }

    // Register a callback for log events
    void registerCallback(const LogCallback& callback) {
        std::lock_guard<std::mutex> lock(mtx);
        callbacks.push_back(callback);
    }

    // Print the latest logs (non-performant, for debugging only)
    void printLatestLogs() const {
        std::lock_guard<std::mutex> lock(mtx);
        for (const auto& log : logs) {
            std::cout << log << std::endl;
        }
    }

private:
    EventLogger() = default; // Private constructor for Singleton
    ~EventLogger() {
        storeLogs(); // Ensure remaining logs are stored on destruction
    }

    // Delete copy constructor and assignment operator
    EventLogger(const EventLogger&) = delete;
    EventLogger& operator=(const EventLogger&) = delete;

    // Store logs to persistent storage (simulated here)
    void storeLogs() {
        // Simulate storing logs (e.g., to a file or database)
        for (const auto& log : logs) {
            // In a real implementation, this would write to a file or database
            std::cout << "Storing log: " << log << std::endl;
        }
        logs.clear(); // Clear logs after storing
    }

    // Get the current timestamp as a string
    std::string getCurrentTimestamp() const {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
        return ss.str();
    }

    std::vector<std::string> logs;               // In-memory log storage
    std::vector<LogCallback> callbacks;          // Observer callbacks
    mutable std::mutex mtx;                              // Mutex for thread safety
    static constexpr size_t logInterval = 10;    // Store logs every 10 entries
};