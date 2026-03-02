#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <xev/shader.h>
#include <fstream>
#include <filesystem>
#include <stdexcept>

TEST_CASE("Shader loading with non-existent file") {
    xev::Shader shader;
    
    // xev::Shader::load logs an error and returns early without crashing.
    REQUIRE_NOTHROW(shader.load(VK_NULL_HANDLE, "this_file_does_not_exist.spv", xev::S_GRAPHICS));
    
    CHECK(shader.get_shader_type() == xev::S_NULL);
}

TEST_CASE("Shader loading with empty file handles gracefully") {
    // Create an empty dummy file
    std::ofstream("empty_shader.spv").close();

    xev::Shader shader;
    // An empty file has size 0. 
    // create_shader() checks if size == 0 and returns VK_NULL_HANDLE before calling Vulkan.
    // It should not crash, and m_shader_type should remain S_NULL.
    shader.load(VK_NULL_HANDLE, "empty_shader.spv", xev::S_GRAPHICS);

    CHECK(shader.get_shader_type() == xev::S_NULL);

    std::filesystem::remove("empty_shader.spv");
}
