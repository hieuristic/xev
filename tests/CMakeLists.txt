add_executable(test_shader test_shader.cpp)

# Include the local doctest directory
target_include_directories(test_shader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(test_shader PRIVATE xev_core)

add_test(NAME test_shader COMMAND test_shader)
