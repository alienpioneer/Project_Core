/**
 * @file test_stress_logger.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include "utils/logger.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <thread>
#include <vector>
#include <atomic>
#include <algorithm>

namespace TestsLogger
{

    class LoggerMacroStressTest : public ::testing::Test 
{

protected:
    void SetUp() override 
    {
        // clean up old logs
        for (int i = 0; i < 10; ++i) 
        {
            std::string name = "app.log" + (i == 0 ? "" : std::to_string(i));

            if (std::filesystem::exists(name))
            {
                std::filesystem::remove(name);
            }
        }

        Utils::Logger::instance().setMaxSize(10 * 1024); // 10 KB for fast rotation

        Utils::Logger::setLevel(Utils::LogLevel::Trace);

        Utils::Logger::enableConsole(false);
    }

    void TearDown() override 
    {
        SetUp(); // cleanup again
    }

    int maxFiles_{10};
};

TEST_F(LoggerMacroStressTest, MultiThreadedLoggingWithMacros) 
{
    constexpr int numThreads = 8;
    constexpr int messagesPerThread = 500;

    std::atomic<int> counter{0};
    std::vector<std::thread> threads;

    auto worker = [&counter](int threadId) {
        for (int i = 0; i < messagesPerThread; ++i) 
        {
            LOG_INFO("Thread " + std::to_string(threadId) + " message " + std::to_string(i));

            counter.fetch_add(1, std::memory_order_relaxed);
        }
    };

    for (int t = 0; t < numThreads; ++t) 
    {
        threads.emplace_back(worker, t);
    }

    for (auto& th : threads) 
    {
        th.join();
    }

    // All messages attempted
    EXPECT_EQ(counter.load(), numThreads * messagesPerThread);

    // Check at least one rotation happened
    bool rotated = false;

    for (int i = 1; i < 5; ++i) 
    {
        if (std::filesystem::exists("app.log" + std::to_string(i))) 
        {
            rotated = true;
            break;
        }
    }

    EXPECT_TRUE(rotated);

    // Current log file exists
    EXPECT_TRUE(std::filesystem::exists("app.log"));

    // Total lines match expected
    int totalLines = 0;

    for (int i = 0; i < 5; ++i) 
    {
        std::string fname = "app.log" + (i == 0 ? "" : std::to_string(i));

        if (!std::filesystem::exists(fname)) continue;

        std::ifstream file(fname);

        totalLines += std::count(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), '\n');
    }

    constexpr int averageLineSize = 26;
    const std::size_t approxLinesPerFile = Utils::Logger::instance().getMaxSize() / averageLineSize;
    const std::size_t maxExpectedLines = approxLinesPerFile * maxFiles_;

    EXPECT_GT(totalLines, 0);
    EXPECT_LE(totalLines, maxExpectedLines);
}

}// namespace TestsLogger
