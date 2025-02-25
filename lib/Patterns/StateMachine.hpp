/**
 * @file StateMachine.hpp
 * @brief this pattern is meant to drive the state of an object or system. this acts as main pattern that will
 * dictate the whole system, committing towards a design that is driven by state, hence trackable by nature, and
 * (hopefully) predictable if any errors occure.
 * 
 */

#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

template<typename stateType_t>
class StateMachine {
public:
    virtual ~StateMachine() = default;

    using StateChangeCallback = std::function<void(stateType_t, stateType_t)>;

    // Register a valid state transition with a specific callback
    void registerStateTransition(stateType_t from, stateType_t to, StateChangeCallback callback) {
        stateTransitions[from].push_back(to);
        stateChangeCallbacks[{from, to}] = callback;
    }

    // Register an observer to be notified of all state changes
    void registerObserver(std::shared_ptr<Observer<stateType_t>> observer) {
        observers.push_back(observer);
    }

    // Change to a new state (if the transition is valid)
    bool changeToState(stateType_t newState) {
        if (isValidTransition(currentState, newState)) {
            stateType_t oldState = currentState;
            currentState = newState;
            notifyStateChange(oldState, newState);
            return true;
        }
        return false; // Invalid transition
    }

    // Get the current state
    stateType_t getState() const {
        return currentState;
    }

protected:
    // Check if a transition is valid
    bool isValidTransition(stateType_t from, stateType_t to) const {
        auto it = stateTransitions.find(from);
        if (it != stateTransitions.end()) {
            for (const auto& validState : it->second) {
                if (validState == to) {
                    return true;
                }
            }
        }
        return false;
    }

    // Notify the registered callback for this specific state change
    void notifyStateChange(stateType_t oldState, stateType_t newState) {
        // Notify the state machine implementer's callback
        auto callbackIt = stateChangeCallbacks.find({oldState, newState});
        if (callbackIt != stateChangeCallbacks.end()) {
            callbackIt->second(oldState, newState);
        }

        // Notify all registered observers
        for (auto& observer : observers) {
            observer->notify(oldState, newState);
        }
    }

    stateType_t currentState;
    std::unordered_map<stateType_t, std::vector<stateType_t>> stateTransitions;
    std::unordered_map<std::pair<stateType_t, stateType_t>, StateChangeCallback> stateChangeCallbacks;
    std::vector<std::shared_ptr<Observer<stateType_t>>> observers;
};