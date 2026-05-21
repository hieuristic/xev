#include <xev/backend.h>
#include <xev/global_descriptor_set.h>

namespace xev {

GlobalDescriptorSet::GlobalDescriptorSet(VkDevice device) : m_device(device) {
  VkResult res_;

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
    m_backend->create_descriptor_pool(m_pool, info);

    res_ = vkCreateDescriptorPool(m_device, &info, nullptr, &pool);
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
    m_backend->create_descriptor_set_layout(m_layout, info);
  }

  {
    VkDescriptorSetAllocateInfo info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &m_layout,
    };
    m_backend->create_descriptor_set(m_set, info);
  }
}

GlobalDescriptorSet::~GlobalDescriptorSet() {
  m_backend->destroy_descriptor_set(m_pool, m_set);
  m_backend->destroy_descriptor_set_layout(m_layout);
  m_backend->destroy_descriptor_pool(m_pool);
}

}  // namespace xev
