cmake_minimum_required(VERSION 3.22)
project(choo)

# Main Executable
file(GLOB SOURCE_FILES src/*.cpp)

add_executable(choo ${SOURCE_FILES})
target_include_directories(choo PRIVATE inc)
target_link_libraries(choo PRIVATE xev_core)
