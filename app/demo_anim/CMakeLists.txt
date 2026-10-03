cmake_minimum_required(VERSION 3.22)
project(demo_anim)

# Main Executable
file(GLOB SOURCE_FILES src/*.cpp)

add_executable(demo_anim ${SOURCE_FILES})
target_include_directories(demo_anim PRIVATE inc)
target_link_libraries(demo_anim PRIVATE xev_core)
xev_add_app(demo_anim)
