#pragma once
#include <vector>
#include <memory>

class Observable;

class Observer {
public:
    virtual void notify(std::weak_ptr<Observable> observable) = 0;
};

class Observable : public std::enable_shared_from_this<Observable> {
public:
    void addObserver(std::shared_ptr<Observer> observer) {
        observerList.push_back(observer);
    }

    void notifyObservers() {
        auto self = shared_from_this();
        for (auto observer : observerList) {
            observer->notify(self);
        }
    }
private:
    std::vector<std::shared_ptr<Observer>> observerList;
};
