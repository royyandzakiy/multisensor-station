#pragma once
#include <Observer.hpp>

// #include <EventLogger.hpp>

class StatefulObjectBase : public Observable {
public:
    virtual ~StatefulObjectBase() = default;
    virtual std::string getId() const = 0;
    virtual std::string getStateAsString() const = 0;
};

template <typename T>
// class StatefulObject: public Observable, public StatefulObjectBase {
class StatefulObject: public Observable {
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

    std::string getId() const {
        return id;
    }

    // Get the current state
    T getState() const {
        return state;
    }

    virtual std::string getStateAsString() const {
        return "ERROR: NO_STATE_TO_STRING_CONVERSION_DEFINED";
    }

private:
    std::string id;  // Unique identifier for the object
    T state;         // Current state
};