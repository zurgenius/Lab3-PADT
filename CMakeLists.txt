cmake_minimum_required(VERSION 3.20)

project(lab2 LANGUAGES CXX)

include(FetchContent)

option(LAB2_ENABLE_WERROR "Treat warnings as errors" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(LAB2_WARNING_FLAGS
    -Wall
    -Wextra
    -Wpedantic
)

if(LAB2_ENABLE_WERROR)
    list(APPEND LAB2_WARNING_FLAGS -Werror)
endif()

add_library(lab2_warnings INTERFACE)
target_compile_options(lab2_warnings INTERFACE ${LAB2_WARNING_FLAGS})

add_library(lab2_core STATIC
    src/bit_sequence.cpp
)
target_include_directories(lab2_core PUBLIC include)
target_link_libraries(lab2_core PUBLIC lab2_warnings)

add_executable(lab2_program
    src/main.cpp
    src/menu.cpp
)
set_target_properties(lab2_program PROPERTIES OUTPUT_NAME program)
target_include_directories(lab2_program PRIVATE include)
target_link_libraries(lab2_program PRIVATE lab2_core lab2_warnings)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
include(GoogleTest)

add_executable(lab2_tests
    tests/tests.cpp
)
set_target_properties(lab2_tests PROPERTIES OUTPUT_NAME tests_runner)
target_include_directories(lab2_tests PRIVATE include)
target_link_libraries(lab2_tests PRIVATE lab2_core lab2_warnings GTest::gtest_main)

gtest_discover_tests(lab2_tests)
