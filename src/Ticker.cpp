#include "Ticker.hpp"

void Ticker::start() {
            thread = std::thread([this]() {
                for (int i=0; i<maxTick; i++) {
                    {
                        std::lock_guard<std::mutex> lock(mtx);
                        sharedCount++;
                        printf("Thread: Shared Count = %d\n", sharedCount);
                    }
                    std::this_thread::sleep_for(threadDelay);
                }
            });
        }

void Ticker::join() {
    if (thread.joinable()) {
        thread.join();
    }
}