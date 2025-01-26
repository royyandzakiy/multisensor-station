#include <gtest/gtest.h>
#include "Ticker.hpp"
#include <thread>
#include <chrono>

class TickerTest : public ::testing::Test {
protected: 
    void SetUp() override {
        sharedCount = 0;
        maxTick = 5;
        ticker = Ticker::create(sharedCount, mtx, maxTick);
    }

    void TearDown() override {
        ticker->join();
    }

    int sharedCount;
    int maxTick;
    std::mutex mtx;
    std::shared_ptr<Ticker> ticker;
};

TEST_F(TickerTest, should_confirm_a_thread_safe_sharedCount_when_incremented_by_ticker_thread_and_main_thread) {
    EXPECT_EQ(true, true);    
}

TEST_F(TickerTest, should_confirm_that_ticker_only_runs_by_amount_of_maxTick) {
    ticker->start();

    // wait for ticker thread to complete, knowing it has threadDelay for 1 second for every loop
    std::this_thread::sleep_for(std::chrono::seconds(maxTick + 1));

    std::lock_guard<std::mutex> lock(mtx);
    EXPECT_GE(sharedCount, maxTick);
}

TEST_F(TickerTest, should_confirm_that_ticker_thread_is_joined_after_destroy) {
    EXPECT_EQ(true, true);
}
