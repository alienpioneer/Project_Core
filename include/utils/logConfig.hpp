/**
 * @file logConfig.hpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#pragma once
#include "Config.hpp"

/**
 * @page LoggerConfigPage Logger Configuration
 * 
 * 
 * @brief Compile-time configuration for logging system.
 *
 * ## Overview
 * Defines compile-time logging levels and enables log filtering at compile stage.
 *
 * ## Log Levels
 * - LOG_LEVEL_TRACE
 * - LOG_LEVEL_DEBUG
 * - LOG_LEVEL_INFO
 * - LOG_LEVEL_WARN
 * - LOG_LEVEL_ERROR
 * - LOG_LEVEL_FATAL
 *
 * ## Compile-Time Filtering
 * - `LOG_COMPILED_LEVEL` controls which logs are compiled.
 * - Messages below this level are removed at compile time.
 *
 * Example:
 * ```
 * #define LOG_COMPILED_LEVEL LOG_LEVEL_INFO
 * ```
 * → TRACE and DEBUG logs are not compiled.
 *
 * ## Macros
 * - LOG_TRACE(msg)
 * - LOG_DEBUG(msg)
 * - LOG_INFO(msg)
 * - LOG_WARN(msg)
 * - LOG_ERROR(msg)
 * - LOG_FATAL(msg)
 *
 * Each macro compiles to:
 * - Logging call if enabled
 * - No-op otherwise
 *
 * ## Constraints
 * - Must be defined before including `logger.hpp`
 * - Compile-time filtering reduces runtime overhead to zero
 */

#define LOG_LEVEL_TRACE 0
#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO  2
#define LOG_LEVEL_WARN  3
#define LOG_LEVEL_ERROR 4
#define LOG_LEVEL_FATAL 5

#ifndef LOG_COMPILED_LEVEL
#define LOG_COMPILED_LEVEL LOG_LEVEL_INFO
#endif