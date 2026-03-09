cmake_minimum_required(VERSION 3.22)

file(GLOB SOURCE_FILES src/*.cpp)

add_library(xev_core STATIC ${SOURCE_FILES})
target_include_directories(xev_core PUBLIC inc ../ext/tinygltf)
target_link_libraries(xev_core PUBLIC imgui
  SDL3::SDL3-shared
  Vulkan::Vulkan
  spdlog::spdlog
  volk::volk
  glm::glm
  VulkanMemoryAllocator
)
target_compile_definitions(xev_core PUBLIC VK_NO_PROTOTYPES)

# Compile shaders
file(GLOB SHADER_FILES src/shaders/*.slang)
target_compile_slang_shaders(xev_core ${SHADER_FILES})
