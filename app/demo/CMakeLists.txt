cmake_minimum_required(VERSION 3.22)
project(demo)

# Main Executable
file(GLOB SOURCE_FILES src/*.cpp)

add_executable(demo ${SOURCE_FILES})
target_include_directories(demo PRIVATE inc)
target_link_libraries(demo PRIVATE xev_core)
