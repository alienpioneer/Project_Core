# @file CMakeLists.txt
# @author Alexandru ALEXANDRESCU
# All rights reserved.

cmake_minimum_required(VERSION 3.20)

if (RUN_UNIT_TESTS)
    enable_testing()

    include(FetchContent)

    # Fetch GoogleTest
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.17.0
    )
    
    FetchContent_MakeAvailable(googletest)

    # Include GoogleTests globaly for all tests
    include(GoogleTest)

    if(RUN_BASE_TESTS)
        add_subdirectory(base)
    endif()

    if(RUN_CORE_TESTS)
        add_subdirectory(core)
    endif()

    if(RUN_UTILS_TESTS)
        add_subdirectory(utils)
    endif()

    add_custom_target(run_unit_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --test-dir ${CMAKE_BINARY_DIR}${RUN_TESTS_DIR} -V --output-on-failure
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running all unit tests"
    )   

endif()