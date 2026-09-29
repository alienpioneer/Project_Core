# @file CMakeLists.txt
# @author Alexandru ALEXANDRESCU
# All rights reserved.

cmake_minimum_required(VERSION 3.22)

set(CORE_TARGET_NAME "test_core")

set(CORE_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/core/Event.cpp
    ${CMAKE_SOURCE_DIR}/src/core/ATimer.cpp
    ${CMAKE_SOURCE_DIR}/src/core/FileLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EventManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/ServiceBase.cpp
    ${CMAKE_SOURCE_DIR}/src/core/ServiceBaseQueue.cpp
    ${CMAKE_SOURCE_DIR}/src/core/ServiceManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/ConfigLoader.cpp
    test_file_loader.cpp
    test_event_manager.cpp
    test_service_base_queue.cpp
    test_service_manager.cpp
    test_config_loader.cpp
)

set(STRESS_CORE_SOURCES
    test_stress_event_manager.cpp
    test_stress_service_manager.cpp
    test_stress_service_payload.cpp
    test_stress_timer.cpp
)

if(RUN_STRESS_TESTS)
    list(APPEND CORE_TEST_SOURCES ${STRESS_CORE_SOURCES})
endif()

# Add target executable
add_executable( ${CORE_TARGET_NAME} ${CORE_TEST_SOURCES})

target_include_directories(${CORE_TARGET_NAME} 
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${CORE_TARGET_NAME} PRIVATE gtest_main)

if(CODE_COVERAGE)
    target_compile_options(${CORE_TARGET_NAME} PRIVATE --coverage -O0 -g)
    target_link_options(${CORE_TARGET_NAME} PRIVATE --coverage)
endif()

# Discover tests - IMPORTANT include(GoogleTest) is already done in the parent folder !
gtest_discover_tests(${CORE_TARGET_NAME})