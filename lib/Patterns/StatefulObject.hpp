#pragma once
#include <Observer.hpp>

// Stateful Object Wrapper
template <typename T>
class StatefulObject: Observable {
public:
    StatefulObject(const std::string& id, T initialState)
        : id(id), state(initialState) {}

    // Update the state and log the change
    void setState(const T& newState) {
        if (state != newState) {
            state = newState;
            notifyObservers();
        }
    }

    // Get the current state
    T getState() const {
        return state;
    }

private:
    std::string id;  // Unique identifier for the object
    T state;         // Current state
};