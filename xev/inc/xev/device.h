#pragma once
#include <xev/volk.h>
#include <cstdint>
#include <optional>

struct SDL_Window;

namespace xev {

enum QFAM {
  Q_GRAPHICS,
  Q_PRESENT,
  Q_COMPUTE,
};

struct QueueFamilyEntry {
  uint32_t idx;
  uint32_t cnt;  // number of queues within the family
};

struct QueueFamily {
  std::optional<QueueFamilyEntry> graphics;
  std::optional<QueueFamilyEntry> present;
  std::optional<QueueFamilyEntry> compute;

  bool isComplete() { return graphics.has_value() && present.has_value(); }
};

struct Device {
  Device();
  Device(SDL_Window* window);
  ~Device();

  Device(const Device&) = delete;
  Device& operator=(const Device&) = delete;
  Device(Device&&) = default;
  Device& operator=(Device&&) = default;

  VkInstance instance{VK_NULL_HANDLE};
  VkSurfaceKHR surface{VK_NULL_HANDLE};
  VkPhysicalDevice physical_device{VK_NULL_HANDLE};
  VkDevice device{VK_NULL_HANDLE};
  QueueFamily queue_family{};

  VkQueue graphics_queue{VK_NULL_HANDLE};
  VkQueue present_queue{VK_NULL_HANDLE};
  VkQueue compute_queue{VK_NULL_HANDLE};

 private:
  void init_instance();
  void pick_physical_device();
  void init_logical_device();
  void find_queue_family();
  void init_surface(SDL_Window* window);
  void init_queues();
};

}  // namespace xev
