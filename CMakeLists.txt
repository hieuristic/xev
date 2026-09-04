cmake_minimum_required(VERSION 3.22)
project(xev LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
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

# Set output directories for libraries
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include custom cmake modules
include(cmake/shader.cmake)
include(cmake/xev_app.cmake)
include(cmake/xev_tool.cmake)

# volk
add_subdirectory(ext/volk EXCLUDE_FROM_ALL)
add_subdirectory(ext/glm EXCLUDE_FROM_ALL)
add_subdirectory(ext/vma EXCLUDE_FROM_ALL)

# GameNetworkingSockets (valvesockets)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_TOOLS OFF CACHE BOOL "" FORCE)
add_subdirectory(ext/valvesockets EXCLUDE_FROM_ALL)

# Jolt Physics
set(TARGET_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(TARGET_HELLO_WORLD OFF CACHE BOOL "" FORCE)
set(TARGET_PERFORMANCE_TEST OFF CACHE BOOL "" FORCE)
set(TARGET_SAMPLES OFF CACHE BOOL "" FORCE)
set(TARGET_VIEWER OFF CACHE BOOL "" FORCE)
set(ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(OVERRIDE_CXX_FLAGS OFF CACHE BOOL "" FORCE)
set(JPH_USE_DX12 OFF CACHE BOOL "" FORCE)
set(JPH_USE_VK OFF CACHE BOOL "" FORCE)
set(JPH_USE_MTL OFF CACHE BOOL "" FORCE)
add_subdirectory(ext/jolt/Build EXCLUDE_FROM_ALL)



# Engine
add_subdirectory(xev)

# Apps (enabled via -DMAKE_APP_<NAME>=ON)
option(MAKE_APP_DEMO "Build the demo app" OFF)
option(MAKE_APP_CHOO "Build the choo app" OFF)

if(MAKE_APP_DEMO)
    add_subdirectory(app/demo)
endif()
if(MAKE_APP_CHOO)
    add_subdirectory(app/choo)
endif()

# Tools (enabled via -DMAKE_TOOL_<NAME>=ON)
option(MAKE_TOOL_PAXER "Build the PaXer tool" OFF)

if (MAKE_TOOL_PAXER)
  add_subdirectory(tool/paxer)
endif()

# Testing
enable_testing()
add_subdirectory(tests)
