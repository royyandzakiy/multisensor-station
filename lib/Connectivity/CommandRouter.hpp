/**
 * @file CommandRouter.hpp
 * @brief this pattern is used to decouple the sender and receiver of a command. 
 * the main idea is to bind a command to a handler and then route the command to its handler.
 * 
 * the pattern shows that command_t will capture all kinds of command, hence centralizing the command handling.
 * the downside is that the command_t enum will grow as the number of commands grow. and this might lead to a
 * large switch statement in the CommandRouter class. Or too many contexts to handle.
 * 
 */

#pragma once

#include <functional>
#include <map>
#include "esp_log.h"

enum class command_t {
    TURN_ON_LED,
    TURN_OFF_LED,
    START_MOTOR,
    STOP_MOTOR,
    UNKNOWN_COMMAND
};

// Forward declarations for dependencies
class LEDManager;
class MotorController;

class CommandRouter {
public:
    // Delete copy constructor and assignment operator to enforce singleton
    CommandRouter(const CommandRouter&) = delete;
    CommandRouter& operator=(const CommandRouter&) = delete;

    // Singleton instance accessor
    static CommandRouter& getInstance() {
        static CommandRouter instance;
        return instance;
    }

    // Bind a command to a handler
    void bindCommand(command_t command, std::function<void()> handler) {
        commandHandlers[command] = handler;
    }

    // Route a command to its handler
    void routeCommand(command_t command) {
        auto it = commandHandlers.find(command);
        if (it != commandHandlers.end()) {
            it->second(); // Invoke the bound handler
        } else {
            // Handle unknown command
            ESP_LOGE("CommandRouter", "Unknown command received");
        }
    }

private:
    // Private constructor for singleton
    CommandRouter() {}

    // Map to store command-handler bindings
    std::map<command_t, std::function<void()>> commandHandlers;
};

// COMMANDERS
class CommandInterface {
public:
    virtual ~CommandInterface() = default;
    virtual command_t getCommand() const = 0; // Get the command from the interface
};

class BLESource : public CommandInterface {
public:
    command_t getCommand() const override {
        // Simulate receiving a command from BLE
        return command_t::TURN_ON_LED;
    }
};

class WiFiSource : public CommandInterface {
public:
    command_t getCommand() const override {
        // Simulate receiving a command from Wi-Fi
        return command_t::START_MOTOR;
    }
};

class MQTTSource : public CommandInterface {
public:
    command_t getCommand() const override {
        // Simulate receiving a command from MQTT
        return command_t::TURN_OFF_LED;
    }
};

// COMMANDABLES
#include "esp_log.h"

class LEDManager {
public:
    void turnOn() {
        // Implementation for turning on LED
        ESP_LOGI("LEDManager", "LED turned on");
    }

    void turnOff() {
        // Implementation for turning off LED
        ESP_LOGI("LEDManager", "LED turned off");
    }
};

class MotorController {
public:
    void start() {
        // Implementation for starting motor
        ESP_LOGI("MotorController", "Motor started");
    }

    void stop() {
        // Implementation for stopping motor
        ESP_LOGI("MotorController", "Motor stopped");
    }
};

// MAIN
// extern "C" void app_main() {
extern "C" void the_main() {
    // Create instances of dependencies
    LEDManager ledManager;
    MotorController motorController;

    // Get the singleton instance of CommandRouter
    CommandRouter& router = CommandRouter::getInstance();

    // Bind commands to their respective handlers
    router.bindCommand(command_t::TURN_ON_LED, std::bind(&LEDManager::turnOn, &ledManager));
    router.bindCommand(command_t::TURN_OFF_LED, std::bind(&LEDManager::turnOff, &ledManager));
    router.bindCommand(command_t::START_MOTOR, std::bind(&MotorController::start, &motorController));
    router.bindCommand(command_t::STOP_MOTOR, std::bind(&MotorController::stop, &motorController));

    // Simulate receiving commands from different sources
    BLESource bleSource;
    WiFiSource wifiSource;
    MQTTSource mqttSource;

    // Route commands from sources
    router.routeCommand(bleSource.getCommand());
    router.routeCommand(wifiSource.getCommand());
    router.routeCommand(mqttSource.getCommand());
}