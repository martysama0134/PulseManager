cmake_minimum_required(VERSION 3.14)
project(PulseManager LANGUAGES CXX)

# Header-only library
add_library(PulseManager INTERFACE)
target_compile_features(PulseManager INTERFACE cxx_std_20)
target_include_directories(PulseManager INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Tests (only when built standalone, not as a subdirectory)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    enable_testing()
    add_executable(test_runner test.cpp)
    target_link_libraries(test_runner PRIVATE PulseManager)
    add_test(NAME PulseManagerTests COMMAND test_runner)
endif()
