/**
 * @file logger.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include "utils/logger.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>

namespace Utils
{

Logger::Logger(std::string baseName, std::string basePath, OutputMode mode)
    :   baseName_{std::move(baseName)},
        basePath_{std::move(basePath)},
        outputMode_{mode}
{
    if (outputMode_ == OutputMode::File)
    {
        openFile();
    }
}

void Logger::init(std::string baseName, std::string basePath, OutputMode mode) 
{
    std::lock_guard<std::mutex> lock(initMutex_);
        
    if(initialized_)
    {
        return;
    }

    initialBaseName_ = std::move(baseName);
    initialBasePath_ = std::move(basePath);
    initialOutputMode_ = mode;
    initialized_ = true;
}

Logger& Logger::instance() 
{
    static Logger instance(getInitialName(), getInitialPath(), getInitialOutput());
    return instance;
}

void Logger::logImpl(LogLevel level, const std::string& msg) 
{
    std::lock_guard<std::mutex> lock(mutex_);

    const std::string ts = makeTimestamp();

    switch (outputMode_)
    {
// Avoid coverage for this snippet        
#ifndef UNIT_TEST

        case OutputMode::Stdout:
        {
            if (level == LogLevel::Fatal || level == LogLevel::Error)
            {
                std::cerr << '[' << ts << "] [" << toString(level) << "] " << msg << '\n';
            }
            else
            {
                std::cout << '[' << ts << "] [" << toString(level) << "] " << msg << '\n';
            }

            if (level >= LogLevel::Debug)
            {
                std::cout.flush();
            }
        }
            break;

        case OutputMode::Stderr:
        {
            std::cerr << '[' << ts << "] [" << toString(level) << "] " << msg << '\n';
            if (level >= LogLevel::Debug)
            {
                std::cout.flush();
            }
        }
            break;
#endif
        case OutputMode::File:
        {
            const std::string line = '[' + ts + "] [" + toString(level) + "] " + msg + '\n';
            rotateFile(line.size());
            file_ << line;
            file_.flush();

            if (consoleEnabled_.load(std::memory_order_relaxed))
            {
                std::cout << '[' << ts << "] [" << toString(level) << "] " << msg << '\n';

                if (level >= LogLevel::Debug)
                {
                    std::cout.flush();
                }
            }
        }
            break;

        default:
            break;
    }
}

void Logger::rotateFile(std::size_t incoming) 
{
    if (currentSize() + incoming <= maxSize_)
    {
        return;
    }

    file_.close();

    for (int i = maxFiles_ - 1; i >= 0; --i) 
    {
        std::string src = basePath_ + baseName_ + ".log" + (i == 0 ? "" : std::to_string(i));
        std::string dst = basePath_ + baseName_ + ".log" + std::to_string(i + 1);

        if (std::filesystem::exists(src)) 
        {
            std::filesystem::rename(src, dst);
        }
    }

    openFile();
}

std::string Logger::makeTimestamp() const 
{
    using namespace std::chrono;

    const auto now = system_clock::now();
    const auto secs = time_point_cast<std::chrono::seconds>(now);
    const auto ms   = duration_cast<milliseconds>(now - secs).count();

    std::time_t tt = system_clock::to_time_t(secs);
    std::tm tm{};
    localtime_r(&tt, &tm);   // use localtime_s on Windows

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

    if (msEnabled_.load(std::memory_order_relaxed)) 
    {
        oss << '.' << std::setw(3) << std::setfill('0') << ms;
    }

    return oss.str();
}

void Logger::log(LogLevel level, const std::string& msg) 
{
    if (level < instance().level_.load(std::memory_order_relaxed))
    {
        return;
    }
    
    instance().logImpl(level, msg);
}

void Logger::openFile() 
{
    file_.open(basePath_ + baseName_ + ".log", std::ios::app);
}

std::size_t Logger::currentSize() const 
{
    const std::string path = basePath_ + baseName_ + ".log";

    if (!std::filesystem::exists(path))
    {
        return 0;
    }

    return std::filesystem::file_size(path);
}

void Logger::enableMilliseconds(bool enable)
{
    instance().msEnabled_.store(enable, std::memory_order_relaxed);
}

void Logger::enableConsole(bool enable) 
{
    instance().consoleEnabled_.store(enable, std::memory_order_relaxed);
}

void Logger::setLevel(LogLevel level) 
{
    instance().level_.store(level, std::memory_order_relaxed);
}

LogLevel Logger::getLevel() const
{
    return level_.load();
}

OutputMode Logger::getOutputMode() const
{
    return outputMode_;
}

bool Logger::getMillisecondsEnabled() const
{
    return msEnabled_.load();
}

const std::string& Logger::getInitialName()
{
    static std::string defaultName = "app";
    return initialBaseName_.empty() ? defaultName : initialBaseName_;
}

const std::string& Logger::getInitialPath()
{
    return initialBasePath_;
}

OutputMode Logger::getInitialOutput()
{
    return initialOutputMode_;
}

void Logger::setMaxSize(std::size_t s) 
{
    std::lock_guard<std::mutex> lock(mutex_);
    maxSize_ = s;
}

std::size_t Logger::getMaxSize() const
{
    return maxSize_;
}

const std::string& Logger::getName() const
{
    return baseName_;
}

const std::string& Logger::getPath() const
{
    return basePath_;
}

void Logger::info(const std::string& m)
{ 
    log(LogLevel::Info,  m); 
}

void Logger::warn(const std::string& m)
{ 
    log(LogLevel::Warn,  m);
}

void Logger::error(const std::string& m)
{ 
    log(LogLevel::Error, m); 
}

void Logger::debug(const std::string& m) 
{ 
    log(LogLevel::Debug, m); 
}

const char* Logger::toString(LogLevel level) 
{
    switch (level) 
    {
        case LogLevel::Trace: return "TRACE";
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info:  return "INFO";
        case LogLevel::Warn:  return "WARN";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Fatal: return "FATAL";
    }
    return "UNKNOWN";
}

}// namespace Utils