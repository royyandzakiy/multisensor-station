#pragma once
#include <Observer.hpp>
#include <EventLogger.hpp>

template <typename T>
class StatefulObject: public Observable {
public:
    StatefulObject(const std::string& id, T initialState)
        : id(id), state(initialState) {}

    // Update the state and log the change
    virtual void setState(const T& newState) {
        if (state != newState) {
            state = newState;
            // notifyObservers();
        }
    }

    std::string getId() const {
        return id;
    }

    // Get the current state
    T getState() const {
        return state;
    }

protected:
    std::string id;  // Unique identifier for the object
    T state;         // Current state
};

template <typename T>
class StatefulObjectLogged : public StatefulObject<T> {
public:
    StatefulObjectLogged(const std::string& id, T initialState): StatefulObject<T>(id, initialState) {}

    void setState(const T& newState) override {
        auto& logger = EventLogger::getInstance();
        if (this->state != newState) {
            this->state = newState;
            // this->notifyObservers();
            // logger.logStateChange(this->id, this->getStateAsString());
        }
    }

    virtual std::string getStateAsString() const = 0;

private:
    std::string id;  // Unique identifier for the object
    T state;         // Current state
};