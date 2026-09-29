# @file project_config.cmake
# @author Alexandru ALEXANDRESCU
# All rights reserved.

cmake_minimum_required(VERSION 3.22)

# ==================================== CORE CONFIG ====================================

#SERVICES
set(EVENTS_QUEUE_MAX_SIZE 100000)

set(SERVICE_QUEUE_MAX_SIZE 10000)

set(MAX_SERVICES 100)

set(MAX_EVENTS 1000)

# FILE READER
# The maximum number of directories and subdirectories to be parsed at once (1-1000)
set(MAX_DIRECTORY_TREE 100)

# Set the maximum size for entry files - default 10MB = 10485760
# The value is for static allocation so keep it small (up to 20-30MB)
set(FILE_MAX_SIZE 104857600)

# CONFIG LOADER

# The maximum number of sections from a .ini file (1-300)
# Sections are the [] marked fields - ex [NETWORK]
set(CONFIG_MAX_SECTIONS 100)
# The maximum number of key-value pairs per section (1-1000)
set(CONFIG_SECTION_MAX_SIZE 200)

# LOGGER
# Maximum consecutive log files
set(LOGGER_MAX_FILES 20)

# ==================================== TESTS ==========================================

# Global test enable flags

# Enable static cppcheck analisys
option(RUN_STATIC_ANALYSIS "Enable cppcheck analysis" OFF)
# Enable unit tests globally
option(RUN_UNIT_TESTS "Enable gtest " OFF)
# Enable stress tests globally. Unit tests MUST be enabled
option(RUN_STRESS_TESTS "Enable Stress tests" OFF)

# Specific unit tests enable flags - first RUN_UNIT_TESTS must be enabled
option(RUN_BASE_TESTS "Enable Base tests" ON)
option(RUN_CORE_TESTS "Enable Core tests" ON)
option(RUN_UTILS_TESTS "Enable Utils tests" ON)