cmake_minimum_required(VERSION 3.22)
project(xev LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Vulkan
find_package(Vulkan REQUIRED)

# SDL3
set(SDL_VULKAN ON CACHE BOOL "Enable Vulkan in SDL" FORCE)
set(SDL_SHARED ON CACHE BOOL "Build SDL shared library" FORCE)
set(SDL_STATIC OFF CACHE BOOL "Build SDL static library" FORCE)
add_subdirectory(ext/sdl3 EXCLUDE_FROM_ALL)

# ImGui
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/imgui)
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
)
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
target_link_libraries(imgui PUBLIC SDL3::SDL3-shared Vulkan::Vulkan)

# spdlog
add_subdirectory(ext/spdlog EXCLUDE_FROM_ALL)

# Set output directories for binaries and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include custom shader compilation
include(cmake/shader.cmake)

# volk
add_subdirectory(ext/volk EXCLUDE_FROM_ALL)
add_subdirectory(ext/glm EXCLUDE_FROM_ALL)
add_subdirectory(ext/vma EXCLUDE_FROM_ALL)

add_subdirectory(xev)
add_subdirectory(app/game0)

# Testing
enable_testing()
add_subdirectory(tests)
