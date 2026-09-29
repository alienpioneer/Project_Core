/**
 * @file test_adata.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include "core/AData.hpp"

#include <gtest/gtest.h>
#include <thread>
#include <atomic>

namespace TestsAData
{

TEST(ADataTest, GetSetBasic)
{
    Core::AData<int> data{42};

    EXPECT_EQ(data.get(), 42);

    data.set(7);
    EXPECT_EQ(data.get(), 7);
}

TEST(ADataTest, TransformUpdatesValue)
{
    Core::AData<int> data{10};

    data.transform([](int& v) {
        v += 5;
    });

    EXPECT_EQ(data.get(), 15);
}

TEST(ADataTest, ReadIsReadOnly)
{
    Core::AData<int> data{3};

    int observed = 0;
    data.read([&](const int& v) 
    {
        observed = v;
    });

    EXPECT_EQ(observed, 3);
    EXPECT_EQ(data.get(), 3);
}

TEST(ADataTest, ShutdownBlocksSet)
{
    Core::AData<int> data{1};

    data.shutdown();
    data.set(99);

    EXPECT_EQ(data.get(), 1);
}

TEST(ADataTest, ShutdownBlocksTransform)
{
    Core::AData<int> data{5};

    data.shutdown();
    data.transform([](int& v) {
        v = 42;
    });

    EXPECT_EQ(data.get(), 5);
}

TEST(ADataTest, ReadAfterShutdownWorks)
{
    Core::AData<int> data{8};

    data.shutdown();

    EXPECT_EQ(data.get(), 8);

    int observed = 0;
    data.read([&](const int& v) {
        observed = v;
    });

    EXPECT_EQ(observed, 8);
}

TEST(ADataTest, ConcurrentTransform)
{
    Core::AData<int> data{0};

    constexpr int iterations = 10000;

    std::thread t1([&] {
        for (int i = 0; i < iterations; ++i)
        {
            data.transform([](int& v) { ++v; });
        }
    });

    std::thread t2([&] {
        for (int i = 0; i < iterations; ++i)
        {
            data.transform([](int& v) { ++v; });
        }
    });

    t1.join();
    t2.join();

    EXPECT_EQ(data.get(), iterations * 2);
}

TEST(ADataTest, ShutdownStopsFurtherWrites)
{
    Core::AData<int> data{0};
    std::atomic<bool> running{true};

    std::thread writer([&] {
        while (running) 
        {
            data.transform([](int& v) { ++v; });
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    data.shutdown();
    running = false;
    writer.join();

    int valueAfterShutdown = data.get();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_EQ(data.get(), valueAfterShutdown);
}

}// namespace TestsAData

