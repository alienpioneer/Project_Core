# @file CMakeLists.txt
# @author Alexandru ALEXANDRESCU
# All rights reserved.

cmake_minimum_required(VERSION 3.22)

set(UTILS_TARGET_NAME "test_utils")

set(CORE_UTILS_SOURCES
    test_logger.cpp  
    test_file_descriptor.cpp
    test_posix_io.cpp
    test_posix_dir.cpp
    test_socket.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/logger.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/FileDescriptor.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/PosixDir.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/PosixIO.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/Socket.cpp
)

set(STRESS_UTILS_SOURCES
    test_stress_logger.cpp
)

if(RUN_STRESS_TESTS)
    list(APPEND CORE_UTILS_SOURCES ${STRESS_UTILS_SOURCES})
endif()

# Add target executable
add_executable( ${UTILS_TARGET_NAME} ${CORE_UTILS_SOURCES})

target_include_directories(${UTILS_TARGET_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include/utils
)

target_link_libraries(${UTILS_TARGET_NAME} PRIVATE gtest_main)
target_compile_definitions(${UTILS_TARGET_NAME} PRIVATE UNIT_TEST)

if(CODE_COVERAGE)
    target_compile_options(${UTILS_TARGET_NAME} PRIVATE --coverage -O0 -g)
    target_link_options(${UTILS_TARGET_NAME} PRIVATE --coverage)
endif()

# Discover tests - IMPORTANT include(GoogleTest) is already done in the parent folder !
gtest_discover_tests(${UTILS_TARGET_NAME})