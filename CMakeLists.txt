# @file CMakeLists.txt
# @author Alexandru ALEXANDRESCU
# All rights reserved.

cmake_minimum_required(VERSION 3.22)

project(project_core VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configuration variables
set(TARGET_NAME core)
set(EXE_NAME core_test)

set(LIB_INSTALL_DIR ${CMAKE_SOURCE_DIR}/deploy)
set(RUN_TESTS_DIR "/tests")

# Default build type (only if not provided)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# Global warnings (safe)
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
)

# Set global log level
add_compile_definitions(LOG_COMPILED_LEVEL=LOG_LEVEL_RELEASE)

if(CMAKE_CROSSCOMPILING)

    message(STATUS "CMAKE_CROSSCOMPILING = ${CMAKE_CROSSCOMPILING}")

    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O1 -g1 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g0 -DNDEBUG")

    set(LIB_DEPLOY_DIR "${LIB_INSTALL_DIR}/target")
else()

    set(LIB_DEPLOY_DIR "${LIB_INSTALL_DIR}/host")
endif()

# ==================================== CONFIGURATION =================================

include(cmake/project_config.cmake)

configure_file(
    include/core/Config.hpp.in
    ${CMAKE_BINARY_DIR}/generated/Config.hpp
    @ONLY
)

option(CODE_COVERAGE "Enable gcov coverage" OFF)
set(COVERAGE_DIR ${CMAKE_SOURCE_DIR}/deploy/coverage)

# ==================================== MODULES =======================================

include_directories(${CMAKE_SOURCE_DIR}/include
                    ${CMAKE_SOURCE_DIR}/include_commons
                    ${CMAKE_BINARY_DIR}/generated
)

# Main sources
add_subdirectory(src)

# ==================================== TESTS ==========================================

if(NOT CMAKE_CROSSCOMPILING)
    if(CODE_COVERAGE)
        set(RUN_UNIT_TESTS ON CACHE BOOL "" FORCE)
        add_compile_definitions(GCOV_BUILD)
    endif()

    if (RUN_UNIT_TESTS)
        add_subdirectory(tests)
    endif()

    if(RUN_STATIC_ANALYSIS)
        # Try to find cppcheck in PATH
        find_program(CPPCHECK_EXECUTABLE NAMES cppcheck)

        if(CPPCHECK_EXECUTABLE)
            message(STATUS "cppcheck found: ${CPPCHECK_EXECUTABLE}")

            add_custom_target(test_cppcheck
                COMMAND ${CPPCHECK_EXECUTABLE} 
                        --project=${CMAKE_BINARY_DIR}/compile_commands.json
                        --enable=all 
                        --inconclusive 
                        --std=c++17 
                        --suppress=missingIncludeSystem
                        --check-config
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                COMMENT "Running cppcheck static analysis"
                VERBATIM
            )

        else()
            message(STATUS "cppcheck not found. Static analysis target will be skipped.")
        endif()
    endif()

endif()

# ==================================== GCOVERAGE =======================================

if(CODE_COVERAGE AND NOT CMAKE_CROSSCOMPILING)
    find_program(LCOV_EXEC lcov)
    find_program(GENHTML_EXEC genhtml)

    if(NOT LCOV_EXEC OR NOT GENHTML_EXEC)
        message(FATAL_ERROR "lcov/genhtml not found")
    endif()

    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${COVERAGE_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_DIR}

        # Reset counters
        COMMAND ${LCOV_EXEC} --directory . --zerocounters

        # Run tests (adjust target if needed)
        # COMMAND ${CMAKE_COMMAND} --build . --target run_unit_tests
        COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/tests/core ./test_core
        COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/tests/base ./test_base
        COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/tests/utils ./test_utils

        # Capture
        COMMAND ${LCOV_EXEC}
            --capture
            --directory .
            --output-file coverage.info

        # Filter out unwanted paths
        COMMAND ${LCOV_EXEC}
            --extract coverage.info
            "${CMAKE_BINARY_DIR}/generated/*"
            "${CMAKE_SOURCE_DIR}/tests/*"
            "${CMAKE_SOURCE_DIR}/src/*"
            "${CMAKE_SOURCE_DIR}/include/*"
            --output-file coverage_filtered.info

        # Generate HTML directly in deploy
        COMMAND ${GENHTML_EXEC}
            coverage_filtered.info
            --output-directory ${COVERAGE_DIR}

        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating coverage report"
        VERBATIM
    )
endif()
