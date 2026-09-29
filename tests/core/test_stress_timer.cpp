/**
 * @file test_stress_timer.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <thread>
#include "core/ATimer.hpp"

namespace TestStressTimer
{
    
TEST(ATimer, HighFrequencyStress)
{
    std::atomic<int> counter{0};

    Core::ATimer timer(
        std::chrono::milliseconds(1),
        [&](){ counter++; },
        false
    );

    timer.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    timer.stop();

    ASSERT_GT(counter.load(), 50);
}

TEST(ATimer, ScheduleChangeWhileRunning)
{
    std::atomic<int> counter{0};

    Core::ATimer timer(
        std::chrono::milliseconds(100),
        [&](){ counter++; },
        false
    );

    timer.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    timer.scheduleRepeating(
        std::chrono::milliseconds(20),
        [&](){ counter++; }
    );

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    timer.stop();

    ASSERT_GT(counter.load(), 5);
}

TEST(ATimer, ConcurrentStartStop)
{
    std::atomic<int> counter{0};

    Core::ATimer timer(
        std::chrono::milliseconds(10),
        [&](){ counter++; },
        false
    );

    std::thread t1([&]()
    {
        for (int i = 0; i < 20; ++i)
        {
            timer.start();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    });

    std::thread t2([&]()
    {
        for (int i = 0; i < 20; ++i)
        {
            timer.stop();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    });

    t1.join();
    t2.join();

    timer.stop();

    SUCCEED();
}

}// namespace TestStressTimer
