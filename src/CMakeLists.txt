# @file CMakeLists.txt
# @author Alexandru ALEXANDRESCU
# All rights reserved.

cmake_minimum_required(VERSION 3.22)

message(STATUS "Building ${TARGET_NAME} static library ...")

add_library(${TARGET_NAME} STATIC
    core/ATimer.cpp
    core/Event.cpp
    core/EventManager.cpp
    core/ServiceBase.cpp
    core/ServiceBaseQueue.cpp
    core/ServiceManager.cpp
    core/FileLoader.cpp
    core/ConfigLoader.cpp
    utils/logger.cpp
    utils/FileDescriptor.cpp
    utils/PosixIO.cpp
    utils/PosixDir.cpp
    utils/Socket.cpp
)

if (NOT CMAKE_CROSSCOMPILING)
    add_executable(${EXE_NAME} main.cpp)
    target_link_libraries(${EXE_NAME} PRIVATE ${TARGET_NAME})

    if(CODE_COVERAGE)
        target_compile_options(${TARGET_NAME} PRIVATE --coverage -O0 -g)
        target_link_options(${TARGET_NAME} PRIVATE --coverage)
        target_compile_options(${EXE_NAME} PRIVATE --coverage -O0 -g)
        target_link_options(${EXE_NAME} PRIVATE --coverage)
    endif()

endif()

# ==================================== INSTALLATION ===================================

if(NOT CODE_COVERAGE)

    set(LIB_NAME "${LIB_DEPLOY_DIR}/lib${TARGET_NAME}.a")

    # Install to prebuilt directory
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${LIB_DEPLOY_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${LIB_INSTALL_DIR}/include
        COMMAND ${CMAKE_COMMAND} -E make_directory ${LIB_INSTALL_DIR}/include_commons
        COMMAND ${CMAKE_COMMAND} -E make_directory ${LIB_INSTALL_DIR}/templates
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${TARGET_NAME}> ${LIB_NAME}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/include ${LIB_INSTALL_DIR}/include
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/include_commons/* ${LIB_INSTALL_DIR}/include_commons
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/generated/* ${LIB_INSTALL_DIR}/include
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/examples/templates/* ${LIB_INSTALL_DIR}/templates
    
        COMMENT "Installing lib${TARGET_NAME}.a ..."
    )
endif()