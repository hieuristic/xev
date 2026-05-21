#pragma once
#include <xev/volk.h>

namespace xev {

class Backend;

class GlobalDescriptorSet {
 public:
  GlobalDescriptorSet(VkDevice device);
  static const uint32_t MAX_TEXTURE = 2 << 14;
  static const uint32_t MAX_SAMPLER = 2 << 5;
  enum SamplerType {
    LINEAR_SAMPLER,
    MULTISAMPLE_SAMPLER,
  };
  static const uint8_t TEXTURE_BINDING = 0;
  static const uint8_t SAMPLER_BINDING = 1;

 private:
  VkDevice m_device {VK_NULL_HANDLE};
  VkDescriptorPool m_pool {VK_NULL_HANDLE};
  VkDescriptorSetLayout m_layout {VK_NULL_HANDLE};
  VkDescriptorSet m_set {VK_NULL_HANDLE};
};

}  // namespace xev
