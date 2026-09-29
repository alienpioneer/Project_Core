/**
 * @file test_timer.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <thread>
#include "core/ATimer.hpp"

namespace TimerUnitTest
{
//One-shot fires exactly once
TEST(ATimer, OneShotFiresOnce)
{
    std::atomic<int> counter{0};

    Core::ATimer timer(
        std::chrono::milliseconds(50),
        [&](){ counter++; },
        true
    );

    timer.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    timer.stop();

    ASSERT_EQ(counter.load(), 1);
}

// scheduleOnce fires exactly once and overrides previous schedule
TEST(ATimer, ScheduleOnceFiresOnce)
{
    std::atomic<int> counter{0};

    Core::ATimer timer(
        std::chrono::milliseconds(100),
        [&](){ counter++; },
        false
    );

    timer.start();

    timer.scheduleOnce(
        std::chrono::milliseconds(50),
        [&](){ counter++; }
    );

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    timer.stop();

    ASSERT_EQ(counter.load(), 1);
}

//Repeating timer fires multiple times
TEST(ATimer, RepeatingFiresMultipleTimes)
{
    std::atomic<int> counter{0};

    Core::ATimer timer(
        std::chrono::milliseconds(40),
        [&](){ counter++; },
        false
    );

    timer.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(220));

    timer.stop();

    ASSERT_GE(counter.load(), 4);
}

//Stop prevents further callbacks
TEST(ATimer, StopPreventsFurtherCallbacks)
{
    std::atomic<int> counter{0};

    Core::ATimer timer(
        std::chrono::milliseconds(40),
        [&](){ counter++; },
        false
    );

    timer.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(120));

    timer.stop();

    int afterStop = counter.load();

    std::this_thread::sleep_for(std::chrono::milliseconds(120));

    ASSERT_EQ(counter.load(), afterStop);
}

//Timer delay roughly respected
TEST(ATimer, DelayAccuracy)
{
    std::atomic<bool> fired{false};

    auto start = std::chrono::steady_clock::now();

    Core::ATimer timer(
        std::chrono::milliseconds(100),
        [&](){ fired = true; },
        true
    );

    timer.start();

    while (!fired)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    auto end = std::chrono::steady_clock::now();

    timer.stop();

    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    ASSERT_GE(elapsed, 90);
}

//Callback replacement during run
TEST(ATimer, CallbackUpdate)
{
    std::atomic<int> counterA{0};
    std::atomic<int> counterB{0};

    Core::ATimer timer(
        std::chrono::milliseconds(40),
        [&](){ counterA++; },
        false
    );

    timer.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(120));

    timer.setCallback([&]()
    {
        counterB++;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(120));

    timer.stop();

    ASSERT_GT(counterA.load(), 0);
    ASSERT_GT(counterB.load(), 0);
}

//Start called twice should not spawn two timers
TEST(ATimer, StartCalledTwice)
{
    std::atomic<int> counter{0};

    Core::ATimer timer(
        std::chrono::milliseconds(50),
        [&](){ counter++; },
        false
    );

    timer.start();
    timer.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(220));

    timer.stop();

    ASSERT_LT(counter.load(), 10);
}

//Changing schedule while running
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

TEST(ATimer, StopFromCallback_NoDeadlock)
{
    std::atomic<int> counter{0};

    {
        Core::ATimer timer(
            std::chrono::milliseconds(100),[&]()
            {
                counter++;
                timer.stop(); 
            },
            false
        );

        timer.start();

        std::this_thread::sleep_for(std::chrono::milliseconds(250));

        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);

        while (!counter.load() && std::chrono::steady_clock::now() < deadline)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        
    } // destructor — must not deadlock or hang

    EXPECT_EQ(counter.load(), 1);
}

//Destructor stops worker thread
TEST(ATimer, DestructorStopsThread)
{
    std::atomic<int> counter{0};

    {
        Core::ATimer timer(
            std::chrono::milliseconds(30),
            [&](){ counter++; },
            false
        );

        timer.start();

        std::this_thread::sleep_for(std::chrono::milliseconds(120));
    }

    int valueAfterDestroy = counter.load();

    std::this_thread::sleep_for(std::chrono::milliseconds(120));

    ASSERT_EQ(counter.load(), valueAfterDestroy);
}

TEST(ATimer, Destructor_AfterStopFromCallback_NoHang)
{
    std::atomic<bool> called{ false };

    auto timer = std::make_unique<Core::ATimer>(std::chrono::milliseconds(50), nullptr, true);

    timer->setCallback([&]()
    {
        called = true;
        timer->stop();
    });

    timer->start();

    while (!called)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
        
    auto t0 = std::chrono::steady_clock::now();
    timer.reset();
    EXPECT_LT(std::chrono::steady_clock::now() - t0, std::chrono::seconds(2));
}

TEST(ATimer, StopFromCallback_ThenRestart_Works)
{
    std::atomic<int> count{ 0 };
    Core::ATimer timer{ std::chrono::milliseconds(50), nullptr, true };

    timer.setCallback([&](){ timer.stop(); ++count; });
    timer.start();
    while (count < 1) std::this_thread::sleep_for(std::chrono::milliseconds(5));

    timer.setCallback([&](){ ++count; });
    timer.start();
    while (count < 2) std::this_thread::sleep_for(std::chrono::milliseconds(5));

    EXPECT_EQ(count.load(), 2);
}

// Multiple scheduleOnce calls → only last one executes
TEST(ATimer, ScheduleOnceMultipleOnlyLastExecutes)
{
    std::atomic<int> counter{0};

    Core::ATimer timer(
        std::chrono::milliseconds(200),
        [&](){ counter++; },
        false
    );

    timer.start();

    timer.scheduleOnce(std::chrono::milliseconds(150), [&](){ counter++; });
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    timer.scheduleOnce(std::chrono::milliseconds(100), [&](){ counter++; });
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    timer.scheduleOnce(std::chrono::milliseconds(50), [&](){ counter++; });

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    timer.stop();

    ASSERT_EQ(counter.load(), 1);
}

// scheduleOnce canceled by stop → no execution
TEST(ATimer, ScheduleOnceCancelledByStop)
{
    std::atomic<int> counter{0};

    Core::ATimer timer(
        std::chrono::milliseconds(100),
        [&](){ counter++; },
        false
    );

    timer.start();

    timer.scheduleOnce(
        std::chrono::milliseconds(100),
        [&](){ counter++; }
    );

    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    timer.stop();

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    ASSERT_EQ(counter.load(), 0);
}

// scheduleOnce timing upper bound tolerance
TEST(ATimer, ScheduleOnceUpperBoundAccuracy)
{
    std::atomic<bool> fired{false};

    auto start = std::chrono::steady_clock::now();

    Core::ATimer timer(
        std::chrono::milliseconds(200),
        [](){},
        false
    );

    timer.start();

    timer.scheduleOnce(
        std::chrono::milliseconds(100),
        [&](){ fired = true; }
    );

    while (!fired)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    auto end = std::chrono::steady_clock::now();

    timer.stop();

    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    ASSERT_GE(elapsed, 90);   // lower bound
    ASSERT_LE(elapsed, 160);  // upper bound tolerance
}

} // namespace TimerUnitTest

