cmake_minimum_required(VERSION 3.22)

# Main Executable
file(GLOB SOURCE_FILES src/*.cpp)

add_executable(game0 ${SOURCE_FILES})
target_include_directories(game0 PRIVATE inc)
target_link_libraries(game0 PRIVATE xev_core)

# Compile shaders
file(GLOB SHADER_FILES shaders/*.slang)
target_compile_slang_shaders(game0 ${SHADER_FILES})
