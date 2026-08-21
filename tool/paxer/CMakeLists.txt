cmake_minimum_required(VERSION 3.22)
project(paxer)

# Main Executable
file(GLOB SOURCE_FILES src/*.cpp)

add_executable(paxer ${SOURCE_FILES})
target_include_directories(paxer PRIVATE inc)
target_link_libraries(paxer PRIVATE xev_core)
xev_add_tool(paxer)
