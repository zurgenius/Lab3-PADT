cmake_minimum_required(VERSION 3.20)

project(lab2 LANGUAGES CXX)

include(FetchContent)

option(LAB2_ENABLE_WERROR "Treat warnings as errors" OFF)
option(WITH_SPLINE_VIEWER "Build spline viewer graphical application" ON)

set(CMAKE_CXX_STANDARD 20)
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
    src/utils.cpp
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

# Google Test
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
    tests/array_sequence_tests.cpp
    tests/cubic_spline_tests.cpp
    tests/dynamic_array_tests.cpp
    tests/linked_list_tests.cpp
    tests/list_sequence_tests.cpp
)
set_target_properties(lab2_tests PROPERTIES OUTPUT_NAME tests_runner)
target_include_directories(lab2_tests PRIVATE include)
target_link_libraries(lab2_tests PRIVATE lab2_core lab2_warnings GTest::gtest_main)

gtest_discover_tests(lab2_tests)

# Spline Viewer
if(WITH_SPLINE_VIEWER)
    # GLFW
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.9
    )
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)

    # Dear ImGui
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.91.0
    )
    FetchContent_MakeAvailable(imgui)

    # ImPlot
    FetchContent_Declare(
        implot
        GIT_REPOSITORY https://github.com/epezent/implot.git
        GIT_TAG v0.16
    )
    FetchContent_MakeAvailable(implot)

    # Заголовки и исходники для lab2_program
    target_include_directories(lab2_program PRIVATE
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${implot_SOURCE_DIR}
    )

    set(IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
    set(IMPLOT_SOURCES
        ${implot_SOURCE_DIR}/implot.cpp
        ${implot_SOURCE_DIR}/implot_items.cpp
    )

    # Добавляем ВСЕ нужные .cpp, включая ваш графический файл
    target_sources(lab2_program PRIVATE
        ${IMGUI_SOURCES}
        ${IMPLOT_SOURCES}
        src/spline_visualization.cpp
    )

    target_link_libraries(lab2_program PRIVATE glfw)

    # OpenGL
    if(UNIX AND NOT APPLE)
        find_package(OpenGL REQUIRED)
        target_link_libraries(lab2_program PRIVATE OpenGL::GL)
    elseif(WIN32)
        target_link_libraries(lab2_program PRIVATE opengl32)
    elseif(APPLE)
        find_library(OpenGL_LIBRARY OpenGL)
        if(OpenGL_LIBRARY)
            target_link_libraries(lab2_program PRIVATE ${OpenGL_LIBRARY})
        endif()
    endif()
endif()
