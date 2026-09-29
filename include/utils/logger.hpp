/**
 * @file logger.hpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#pragma once

#include "logConfig.hpp"

#include <atomic>
#include <mutex>
#include <string>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>


/**
 * @page LoggerPage Logger
 * @brief Thread-safe singleton logger with file rotation.
 *
 * ## Overview
 * `Logger` provides:
 * - Thread-safe logging
 * - File output with rotation
 * - Optional console output
 * - Configurable log level and formatting
 *
 * ## Initialization
 * ```
 * Logger::init("app", "/var/log/");
 * ```
 * Must be called before first use.
 *
 * ## Usage
 * ```
 * LOG_INFO("Application started");
 * LOG_DEBUG("Value x = 42");
 * ```
 *
 * ## Features
 * - Singleton instance
 * - File rotation with configurable max size
 * - Optional millisecond timestamps
 * - Atomic log level filtering
 *
 * ## Threading
 * - Logging is thread-safe
 * - Internal mutex protects file access
 * - Log level and flags are atomic
 *
 * ## File Rotation
 * - Triggered when file exceeds `maxSize`
 * - Files renamed:
 *   - app.log → app.log1 → app.log2 ...
 *
 * ## Constraints
 * - `init()` must be called before first `instance()`
 * - Instance configuration cannot be changed after creation
 * - No blocking operations outside internal mutex
 * 
 * @warning Current design is intentionally immutable after initialization !
 * 
 *  * Usage is divided into **setup** and **logging**.
 * 
 *    **Setup** (required before using the logger)
 *    @code{.cpp}
 *    Utils::Logger::init("test");                 // set file name / path
 *    Utils::Logger::setLevel(Utils::LogLevel::Debug);
 *    Utils::Logger::instance().setMaxSize(1024);
 *    Utils::Logger::enableConsole(true);
 *    Utils::Logger::enableMilliseconds(true);
 *    @endcode
 * 
 * @warning The `init()` function must be called before accessing `Logger::instance()`.
 * 
 * ## The Logger provides two main usage styles:
 * 
 * 1. **Compile-time macros** (efficient, filtered by LOG_COMPILED_LEVEL)
 * 
 *    Macros allow compile-time filtering of log messages according to `LOG_COMPILED_LEVEL`.
 *    Only messages at or above the compiled level are included in the binary.
 * 
 *    @code{.cpp}
 *    LOG_INFO("Application started");
 *    LOG_DEBUG("Value x = 42");
 *    LOG_ERROR("Something failed");
 *    @endcode
 * 
 * @note Use macros in performance-critical code; runtime API is more flexible but incurs runtime cost.
 * 
 * 2. **Runtime logging API** (less efficient )
 * 
 *    @code{.cpp}
 *    Utils::Logger::info("Application started");
 *    Utils::Logger::debug("Value x = 42");
 *    Utils::Logger::error("Something failed");
 *    @endcode
 * 
 * @warning In this case some lower level logger functions are compiled and called even if the log level doesn't require them ! 
 * 
 * 
 * @section References
 * 
 * - Utils::Logger
 */

namespace Utils
{
    class Logger;
    enum class LogLevel: uint8_t;
    enum class OutputMode;
}

#if LOG_COMPILED_LEVEL <= LOG_LEVEL_TRACE
#define LOG_TRACE(msg) Utils::Logger::log(Utils::LogLevel::Trace, msg)
#else
#define LOG_TRACE(msg) ((void)0)
#endif

#if LOG_COMPILED_LEVEL <= LOG_LEVEL_DEBUG
#define LOG_DEBUG(msg) Utils::Logger::log(Utils::LogLevel::Debug, msg)
#else
#define LOG_DEBUG(msg) ((void)0)
#endif

#if LOG_COMPILED_LEVEL <= LOG_LEVEL_INFO
#define LOG_INFO(msg) Utils::Logger::log(Utils::LogLevel::Info, msg)
#else
#define LOG_INFO(msg) ((void)0)
#endif

#if LOG_COMPILED_LEVEL <= LOG_LEVEL_WARN
#define LOG_WARN(msg) Utils::Logger::log(Utils::LogLevel::Warn, msg)
#else
#define LOG_WARN(msg) ((void)0)
#endif

#if LOG_COMPILED_LEVEL <= LOG_LEVEL_ERROR
#define LOG_ERROR(msg) Utils::Logger::log(Utils::LogLevel::Error, msg)
#else
#define LOG_ERROR(msg) ((void)0)
#endif

#if LOG_COMPILED_LEVEL <= LOG_LEVEL_FATAL
#define LOG_FATAL(msg) Utils::Logger::log(Utils::LogLevel::Fatal, msg)
#else
#define LOG_FATAL(msg) ((void)0)
#endif

namespace Utils
{

/**
 * @enum Utils::LogLevel
 * @brief Runtime log levels.
 */
enum class LogLevel : uint8_t 
{
    Trace = 0, /**< Most verbose logging */
    Debug,     /**< Debug information */
    Info,      /**< General information */
    Warn,      /**< Warning conditions */
    Error,     /**< Error conditions */
    Fatal      /**< Critical failures */
};

/**
 * @enum Utils::OutputMode
 * @brief Runtime output mode.
 */
enum class OutputMode
{
    File,
    Stdout,
    Stderr
};


/**
 * @class Utils::Logger
 * @brief Singleton logger implementation.
 */
class Logger 
{
public:
    /**
     * @brief Returns the singleton instance.
     */
    static Logger& instance();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief Initialize logger before first use.
     * @param baseName Log file base name.
     * @param basePath Optional path for log files.
     * @param mode Set the output mode.
     */
    static void init(std::string baseName, std::string basePath="", OutputMode mode=OutputMode::File) ;

    /**
     * @brief Enable or disable console output along with the file.
     */
    static void enableConsole(bool enable);

    /**
     * @brief Enable or disable millisecond precision in timestamps.
     */
    static void enableMilliseconds(bool enable); 

    /**
     * @brief Check if millisecond precision in timestamp is enabled
     * @note For tests only
     */
    bool getMillisecondsEnabled() const;

    /**
     * @brief Set runtime log level.
     */
    static void setLevel(LogLevel level);

    /**
     * @brief Get log level.
     * @note: For tests only !
     */
    LogLevel getLevel() const;

    /**
     * @brief Get output mode
     * @note: For tests only !
     */
    OutputMode getOutputMode() const;

    /**
     * @brief Log a message at given level.
     */
    static void log(LogLevel level, const std::string& msg);

    /**
     * @brief Set maximum file size before rotation.
     */
    void setMaxSize(std::size_t s);

    /**
     * @brief Get maximum file size.
     */
    std::size_t getMaxSize() const;

    /**
     * @brief Get log base name.
     */
    const std::string& getName() const;

    /**
     * @brief Get log path.
     */
    const std::string& getPath() const;

    /**
     * @brief Log info-level message.
     */
    static void info(const std::string& m);

    /**
     * @brief Log warning-level message.
     */
    static void warn(const std::string& m); 

    /**
     * @brief Log error-level message.
     */
    static void error(const std::string& m);

    /**
     * @brief Log debug-level message.
     */
    static void debug(const std::string& m);

private:

    /**
     * @brief Construct logger instance.
     */
    explicit Logger(std::string baseName, std::string basePath, OutputMode mode);

    /**
     * @brief Internal logging implementation.
     */
    void logImpl(LogLevel level, const std::string& msg);

    /**
     * @brief Open log file.
     */
    void openFile(); 

    /**
     * @brief Create formatted timestamp string.
     */
    std::string makeTimestamp() const ;

    /**
     * @brief Rotate log files when size limit is exceeded.
     */
    void rotateFile(std::size_t incoming);

    /**
     * @brief Get current log file size.
     */
    std::size_t currentSize() const;

    /**
     * @brief Convert log level to string.
     */
    static const char* toString(LogLevel level);

    /**
     * @brief Retrieve initial base name.
     */
    static inline const std::string& getInitialName();

    /**
     * @brief Retrieve initial path.
     */
    static inline const std::string& getInitialPath();

    /**
     * @brief Retrieve initial OutputMode.
     */
    static inline OutputMode getInitialOutput();

private:
    std::string baseName_;                                          /**< Log file base name */
    std::string basePath_;                                          /**< Log file path */
    std::ofstream file_;                                            /**< Output file stream */
    std::mutex mutex_;                                              /**< File access mutex */
    std::size_t maxSize_{4*1024*1024};                              /**< Max file size before rotation */
    OutputMode outputMode_;                                         /**< Output mode for the logger */
    std::atomic<LogLevel> level_{LogLevel::Info};                   /**< Runtime log level */

    static constexpr int maxFiles_ = LOGGER_MAX_FILES;              /**< Max rotated files */

    static inline bool initialized_ = false;                        /**< Init guard */
    static inline std::mutex initMutex_;                            /**< Init mutex */


    std::atomic<bool> consoleEnabled_{true};                        /**< Console output flag */
    std::atomic<bool> msEnabled_{false};                            /**< Millisecond flag */

    static inline std::string initialBaseName_;                     /**< Initial name before instance */
    static inline std::string initialBasePath_;                     /**< Initial path before instance */
    static inline OutputMode initialOutputMode_{OutputMode::File};  /**< Initial path before instance */
};


}// namespace Utils

