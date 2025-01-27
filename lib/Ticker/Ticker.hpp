#pragma once

#include <stdio.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <memory>

class Ticker {
public:
    // Factory method to create a shared_ptr<Ticker>
    static std::shared_ptr<Ticker> create(int& sharedCount, std::mutex& mtx, const int maxTick) {
        return std::shared_ptr<Ticker>(new Ticker(sharedCount, mtx, maxTick));
    }
    ~Ticker() {
        join();
    }

    void start();
    void join();

private:
    // Private constructor to enforce use of factory method
    Ticker(int& sharedCount, std::mutex& mtx, const int maxTick)
        : sharedCount(sharedCount), mtx(mtx), maxTick(maxTick), threadDelay(std::chrono::seconds(1)) {}

    int& sharedCount;
    std::mutex& mtx; 
    int maxTick;     
    std::thread thread;
    std::chrono::seconds threadDelay;
};