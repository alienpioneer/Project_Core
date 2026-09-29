/**
 * @file test_logger.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include "utils/logger.hpp"
#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <regex>

namespace TestsLogger
{

class LoggerTest : public ::testing::Test 
{
protected:
    void SetUp() override 
    {
        // remove any existing test logs
        for (int i = 0; i < 10; ++i) 
        {
            std::string name = "test.log" + (i == 0 ? "" : std::to_string(i));

            if (std::filesystem::exists(name))
            {
                std::filesystem::remove(name);
            }
        }
    }

    void TearDown() override 
    {
        SetUp(); // clean up again
    }
};

class LoggerTimestampTest : public ::testing::Test 
{

private:
    void cleanup()
    {
        // Clean log files
        for (int i = 0; i < 3; ++i) 
        {
            std::string name = "app.log" + (i == 0 ? "" : std::to_string(i));

            if (std::filesystem::exists(name))
            {
                std::filesystem::remove(name);
            }
        }
    }

protected:

    void SetUp() override 
    {
        cleanup();

        Utils::Logger::enableConsole(false);
        Utils::Logger::setLevel(Utils::LogLevel::Info);
        Utils::Logger::instance().setMaxSize(1024 * 1024);
    }

    void TearDown() override 
    {
        cleanup();
    }
};

TEST_F(LoggerTest, LogWritesToFile) 
{
    Utils::Logger::instance(); // ensure initialized
    Utils::Logger::setLevel(Utils::LogLevel::Trace);
    Utils::Logger::enableConsole(false);

    LOG_INFO("Test message");

    std::ifstream file("app.log");
    ASSERT_TRUE(file.is_open());

    std::string line;
    std::getline(file, line);
    ASSERT_NE(line.find("Test message"), std::string::npos);
}

TEST_F(LoggerTest, LogRotationCreatesNewFile) 
{
    Utils::Logger::instance().setMaxSize(1024); // 1 KB for fast rotation
    Utils::Logger::setLevel(Utils::LogLevel::Trace);
    Utils::Logger::enableConsole(false);

    std::string largeMsg(200, 'X'); // 200 bytes per log

    // write enough messages to exceed 1 KB -> rotation
    for (int i = 0; i < 10; ++i) 
    {
        LOG_INFO(largeMsg);
    }

    // The first rotation should create app.log1
    bool rotated = std::filesystem::exists("app.log1");
    EXPECT_TRUE(rotated);

    // The current log file should still exist
    EXPECT_TRUE(std::filesystem::exists("app.log"));
}

TEST_F(LoggerTest, GetMaxSize)
{
    auto& logger = Utils::Logger::instance();

    logger.setMaxSize(1234);

    EXPECT_EQ(logger.getMaxSize(), 1234u);
}

TEST_F(LoggerTest, GetName)
{
    auto& logger = Utils::Logger::instance();

    EXPECT_FALSE(logger.getName().empty());
}

TEST_F(LoggerTest, GetPath)
{
    auto& logger = Utils::Logger::instance();

    EXPECT_TRUE(logger.getPath().empty());
}

TEST_F(LoggerTimestampTest, EnableMillisecondsToggle)
{
    std::filesystem::remove("app.log");

    auto& logger = Utils::Logger::instance();

    logger.enableMilliseconds(true);

    EXPECT_TRUE(logger.getMillisecondsEnabled());
}

TEST_F(LoggerTest, InfoWritesCorrectLevel)
{
    std::filesystem::remove("app.log");

    auto& logger = Utils::Logger::instance();
    logger.setLevel(Utils::LogLevel::Trace);
    logger.enableConsole(false);
    logger.info("info message");
    logger.warn("warn message");
    logger.debug("debug message");
    logger.error("error message");

    EXPECT_EQ(logger.getLevel(), Utils::LogLevel::Trace);
}

// =================================== SPECIFIC TESTS ===================================

// These tests don't work with gcov since they are calling init()
#ifndef GCOV_BUILD

TEST_F(LoggerTest, LogLevelFiltering) 
{
    Utils::Logger::init("testabc");
    Utils::Logger::setLevel(Utils::LogLevel::Error);
    Utils::Logger::enableConsole(false);

    std::cout << "Logger name " << Utils::Logger::instance().getName() << std::endl;

    LOG_DEBUG("Debug message");  // should be filtered
    LOG_ERROR("Error message");  // should be logged

    std::ifstream file("testabc.log");
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    ASSERT_EQ(content.find("Debug message"), std::string::npos);
    ASSERT_NE(content.find("Error message"), std::string::npos);
}

TEST_F(LoggerTimestampTest, TimestampWithoutMilliseconds) 
{
    Utils::Logger::enableMilliseconds(false);

    LOG_INFO("No milliseconds");

    std::ifstream file("app.log");
    ASSERT_TRUE(file.is_open());

    std::string line;
    std::getline(file, line);

    // [YYYY-MM-DD HH:MM:SS] [INFO] ...
    const std::regex pattern(R"(^\[\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\] \[INFO\] )");

    EXPECT_TRUE(std::regex_search(line, pattern));
}

TEST_F(LoggerTimestampTest, TimestampWithMilliseconds) 
{
    Utils::Logger::enableMilliseconds(true);

    LOG_INFO("With milliseconds");

    std::ifstream file("app.log");
    ASSERT_TRUE(file.is_open());

    std::string line;
    std::getline(file, line);

    // Expected [YYYY-MM-DD HH:MM:SS.mmm] [INFO] ...
    const std::regex pattern(R"(^\[\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3}\] \[INFO\] )");

    EXPECT_TRUE(std::regex_search(line, pattern));
}

TEST_F(LoggerTest, InitCalledTwiceIgnored)
{
    Utils::Logger::init("first", "./");
    Utils::Logger::init("second", "/tmp/");

    auto& logger = Utils::Logger::instance();

    EXPECT_EQ(logger.getName(), "first");
    EXPECT_EQ(logger.getPath(), "./");
}

TEST_F(LoggerTest, InitSetsBaseNameAndPath)
{
    Utils::Logger::init("mylog", "./");

    auto& logger = Utils::Logger::instance();

    EXPECT_EQ(logger.getName(), "mylog");
    EXPECT_EQ(logger.getPath(), "./");
}

#endif

}// namespace TestsLogger