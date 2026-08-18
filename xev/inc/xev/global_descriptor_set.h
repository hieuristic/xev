#pragma once
#include <xev/resource/image.h>
#include <xev/resource/sampler.h>
#include <xev/volk.h>
#include <array>

namespace xev {

struct Backend;

struct GlobalDescriptorSet {
  GlobalDescriptorSet(VkDevice device);
  ~GlobalDescriptorSet();
  static const uint32_t MAX_TEXTURE = 2 << 14;
  static const uint32_t MAX_SAMPLER = 2 << 5;
  static const uint8_t SAMPLER_BINDING = 0;
  static const uint8_t TEXTURE_BINDING = 1;

  uint32_t set(const Image& image);
  void set(const Image& image, uint32_t id) const;
  void unset_texture(uint32_t id);
  void set(const Sampler& sampler, uint32_t id) const;

  void bind(VkCommandBuffer cmdbuf, VkPipelineLayout pipeline_layout) const;

  VkDescriptorSetLayout get_layout() const { return m_layout; }

 private:
  VkDevice m_device{VK_NULL_HANDLE};
  VkDescriptorPool m_pool{VK_NULL_HANDLE};
  VkDescriptorSetLayout m_layout{VK_NULL_HANDLE};
  VkDescriptorSet m_set{VK_NULL_HANDLE};

  std::array<uint64_t, 4> free_textures_l1;
  std::array<uint64_t, 256> free_textures_l0;

  std::array<Sampler, 1> m_global_samplers{Sampler{SAMPLER_LINEAR}};
  void alloc_sampler(Sampler& sampler);
  void free_sampler(Sampler& sampler);
  void init_samplers();

  VkSampler linear_sampler;

  uint32_t get_free_texture_id();
  constexpr uint32_t get_free_bit(uint64_t bitset) const;
};

}  // namespace xev
