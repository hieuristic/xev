#pragma once
#include <xev/volk.h>
#include <xev/resource/image.h>
#include <array>

namespace xev {

class Backend;

class GlobalDescriptorSet {
 public:
  GlobalDescriptorSet(VkDevice device);
  ~GlobalDescriptorSet();
  static const uint32_t MAX_TEXTURE = 2 << 14;
  static const uint32_t MAX_SAMPLER = 2 << 5;
  enum SamplerType {
    LINEAR_SAMPLER,
    MULTISAMPLE_SAMPLER,
  };
  static const uint8_t TEXTURE_BINDING = 0;
  static const uint8_t SAMPLER_BINDING = 1;

  uint32_t set(const Image& image);
  void set(const Image& image, uint32_t id);

  void unset_texture(uint32_t id);
  void unset_sampler(uint32_t id);

  VkDescriptorSetLayout get_layout() const { return m_layout; }

 private:
  VkDevice m_device{VK_NULL_HANDLE};
  VkDescriptorPool m_pool{VK_NULL_HANDLE};
  VkDescriptorSetLayout m_layout{VK_NULL_HANDLE};
  VkDescriptorSet m_set{VK_NULL_HANDLE};

  std::array<uint64_t, 4> free_textures_l1;
  std::array<uint64_t, 256> free_textures_l0;
  uint32_t free_samplers;

  uint32_t get_free_texture_id();
  constexpr uint32_t get_free_bit(uint64_t bitset) const;
};

}  // namespace xev
