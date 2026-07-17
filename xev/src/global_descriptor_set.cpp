#include <xev/global_descriptor_set.h>
#include <xev/logger.h>

namespace xev {

GlobalDescriptorSet::GlobalDescriptorSet(VkDevice device) : m_device(device) {
  VkResult res_;

  free_textures_l1.fill(0);
  free_textures_l0.fill(0);

  {  // create descriptor pool
    std::array<VkDescriptorPoolSize, 2> sizes = {{
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, GlobalDescriptorSet::MAX_TEXTURE},
        {VK_DESCRIPTOR_TYPE_SAMPLER, GlobalDescriptorSet::MAX_SAMPLER},
    }};
    VkDescriptorPoolCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT,
        .maxSets = MAX_TEXTURE * 2,
        .poolSizeCount = 2,
        .pPoolSizes = sizes.data(),
    };

    res_ = vkCreateDescriptorPool(m_device, &info, nullptr, &m_pool);
    XEV_ASSERT_VK(res_, "Failed to create descriptor pool");
  }

  {  // creating layout
    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
        {{
             .binding = TEXTURE_BINDING,
             .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
             .descriptorCount = MAX_TEXTURE,
             .stageFlags = VK_SHADER_STAGE_ALL,
         },
         {
             .binding = SAMPLER_BINDING,
             .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
             .descriptorCount = MAX_SAMPLER,
             .stageFlags = VK_SHADER_STAGE_ALL,
         }}};
    std::array<VkDescriptorBindingFlags, 2> flags = {{
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
    }};

    VkDescriptorSetLayoutBindingFlagsCreateInfo flag_info = {
        .sType =
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .bindingCount = 2,
        .pBindingFlags = flags.data(),
    };

    VkDescriptorSetLayoutCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &flag_info,
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
        .bindingCount = 2,
        .pBindings = bindings.data(),
    };
    res_ = vkCreateDescriptorSetLayout(m_device, &info, nullptr, &m_layout);
    XEV_ASSERT_VK(res_, "Failed to create descriptor set layout");
  }

  {
    VkDescriptorSetAllocateInfo info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &m_layout,
    };
    res_ = vkAllocateDescriptorSets(m_device, &info, &m_set);
    XEV_ASSERT_VK(res_, "Failed to create descriptor set.");
  }

  init_samplers();

  XEV_INFO("Created descriptor set!");
}

void GlobalDescriptorSet::set(const Sampler& sampler, uint32_t id) const {
  const VkDescriptorImageInfo info = {
      .sampler = sampler.sampler,
  };
  const VkWriteDescriptorSet write_set = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = m_set,
      .dstBinding = SAMPLER_BINDING,  // binding 1
      .dstArrayElement = id,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
      .pImageInfo = &info,
  };
  vkUpdateDescriptorSets(m_device, 1, &write_set, 0, nullptr);
}

void GlobalDescriptorSet::alloc_sampler(Sampler& sampler) {
  VkSamplerCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .anisotropyEnable = VK_TRUE,
      .maxAnisotropy = MAX_SAMPLER_ANISOTROPY,
  };

  switch (sampler.type) {
    case SamplerType::SAMPLER_LINEAR:
      info.magFilter = VK_FILTER_LINEAR;
      info.minFilter = VK_FILTER_LINEAR;
      info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
      info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
      info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
      break;
  }

  VkResult res_ = vkCreateSampler(m_device, &info, nullptr, &sampler.sampler);
  XEV_ASSERT_VK(res_, "Failed to create sampler");
}

void GlobalDescriptorSet::free_sampler(Sampler& sampler) {
  vkDestroySampler(m_device, sampler.sampler, nullptr);
  sampler.sampler = VK_NULL_HANDLE;
}

void GlobalDescriptorSet::bind(VkCommandBuffer cmdbuf,
                               VkPipelineLayout pipeline_layout) const {
  vkCmdBindDescriptorSets(cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          pipeline_layout, 0, 1, &m_set, 0, nullptr);
}

void GlobalDescriptorSet::init_samplers() {
  for (uint32_t i = 0; i < m_global_samplers.size(); i++) {
    alloc_sampler(m_global_samplers[i]);
    set(m_global_samplers[i], i);
  }
}

GlobalDescriptorSet::~GlobalDescriptorSet() {
  for (auto& sampler : m_global_samplers)
    free_sampler(sampler);

  if (m_pool != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(m_device, m_pool, nullptr);
    m_pool = VK_NULL_HANDLE;
  }

  if (m_layout != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(m_device, m_layout, nullptr);
    m_layout = VK_NULL_HANDLE;
  }
}

uint32_t GlobalDescriptorSet::set(const Image& image) {
  uint32_t id = get_free_texture_id();
  set(image, id);
  return id;
}

void GlobalDescriptorSet::set(const Image& image, uint32_t id) const {
  const VkDescriptorImageInfo info = {
      .imageView = image.view,
      .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
  };
  const VkWriteDescriptorSet write_set = {
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = m_set,
      .dstBinding = TEXTURE_BINDING,
      .dstArrayElement = id,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
      .pImageInfo = &info,
  };
  vkUpdateDescriptorSets(m_device, 1, &write_set, 0, nullptr);
}

constexpr uint32_t GlobalDescriptorSet::get_free_bit(uint64_t bitset) const {
  uint64_t inv = ~bitset;
#if defined(_MSC_VER)
  unsigned long offset;
  _BitScanForward64(&offset, inv);
#else
  uint32_t offset = __builtin_ctzll(inv);
#endif
  return offset;
}

void GlobalDescriptorSet::unset_texture(uint32_t id) {
  uint32_t idx_l1 = id / 64;
  uint32_t free_bit_l1 = idx_l1 % 64;
  if (free_textures_l0[idx_l1] == 0xFFFFFFFFFFFFFFFF)
    free_textures_l1[idx_l1 / 64] &= ~(1ULL << free_bit_l1);
  uint32_t free_bit_l0 = id % 64;
  free_textures_l0[idx_l1] &= ~(1ULL << free_bit_l0);
}

uint32_t GlobalDescriptorSet::get_free_texture_id() {
  bool found = false;
  for (uint32_t i = 0; i < 4; i++) {
    if (free_textures_l1[i] != 0xFFFFFFFFFFFFFFFF) {
      found = true;
      uint32_t free_bit_l1 = get_free_bit(free_textures_l1[i]);
      uint32_t idx_l1 = (i * 64) + free_bit_l1;
      uint32_t free_bit_l0 = get_free_bit(free_textures_l0[idx_l1]);
      uint32_t idx_l0 = idx_l1 * 64 + free_bit_l0;

      free_textures_l0[idx_l1] |= (1ULL << free_bit_l0);
      if (free_textures_l0[idx_l1] == 0xFFFFFFFFFFFFFFFF)
        free_textures_l1[i] |= (1ULL << free_bit_l1);

      return idx_l0;
    }
  }
  XEV_ERROR("GLOBAL DESCRIPTOR SET IS FULL!");
  return 0;
}

}  // namespace xev
