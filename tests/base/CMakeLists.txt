# @file CMakeLists.txt
# @author Alexandru ALEXANDRESCU
# All rights rserved.

cmake_minimum_required(VERSION 3.22)

set(BASE_TARGET_NAME "test_base")

add_executable(${BASE_TARGET_NAME} 
    ${CMAKE_SOURCE_DIR}/src/core/Event.cpp
    ${CMAKE_SOURCE_DIR}/src/core/ATimer.cpp
    test_aqueue.cpp
    test_avector.cpp
    test_event.cpp
    test_adata.cpp
    test_timer.cpp
    test_amap.cpp
)

target_include_directories(${BASE_TARGET_NAME} 
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${BASE_TARGET_NAME}
    PRIVATE
        gtest_main
)

if(CODE_COVERAGE)
    target_compile_options(${BASE_TARGET_NAME} PRIVATE --coverage -O0 -g)
    target_link_options(${BASE_TARGET_NAME} PRIVATE --coverage)
endif()

# Discover tests - IMPORTANT include(GoogleTest) is already done in the parent folder !
gtest_discover_tests(${BASE_TARGET_NAME})