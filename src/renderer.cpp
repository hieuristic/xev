#include <xev/renderer.h>

namespace xev {

Renderer::Renderer(std::shared_ptr<Backend> backend, std::unique_ptr<Shader> shader,
                   const Scene &scene)
    : m_backend(backend), m_shader(std::move(shader)), m_scene(&scene) {

  VkDevice device = m_backend->get_device();
  VkFormat format = m_backend->get_format().format;

  create_pipeline(device, format);

  // 1. Create buffers for scene data
  VkDeviceSize bufferSize = sizeof(glm::vec3) * scene.m_vert_buffer.size();

  if (bufferSize > 0) {
    create_buffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  m_vertex_buffer, m_vertex_buffer_memory);

    void *data;
    vkMapMemory(device, m_vertex_buffer_memory, 0, bufferSize, 0, &data);
    memcpy(data, scene.m_vert_buffer.data(), (size_t)bufferSize);
    vkUnmapMemory(device, m_vertex_buffer_memory);
  }

  m_index_count = scene.m_face_buffer.size() * 3;
  VkDeviceSize idx_buffer_size =
      sizeof(glm::uvec3) * scene.m_face_buffer.size();

  if (idx_buffer_size > 0) {
    create_buffer(idx_buffer_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  m_index_buffer, m_index_buffer_memory);

    void *data;
    vkMapMemory(device, m_index_buffer_memory, 0, idx_buffer_size, 0, &data);
    memcpy(data, scene.m_face_buffer.data(), (size_t)idx_buffer_size);
    vkUnmapMemory(device, m_index_buffer_memory);
  }

  // 2. Command pool & buffer
  std::optional<QueueFamilyEntry> gfx_qfam =
      m_backend->get_queue_family_index(Q_GRAPHICS);
  VkCommandPoolCreateInfo poolInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = gfx_qfam.value().idx,
  };

  if (vkCreateCommandPool(device, &poolInfo, nullptr, &m_cmd_pool) !=
      VK_SUCCESS) {
    XEV_ERROR("Failed to create command pool!");
  }

  VkCommandBufferAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = m_cmd_pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
  };

  if (vkAllocateCommandBuffers(device, &allocInfo, &m_cmd_buffer) !=
      VK_SUCCESS) {
    XEV_ERROR("Failed to allocate command buffers!");
  }

  // 3. Sync objects
  VkSemaphoreCreateInfo semaphoreInfo = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };

  VkFenceCreateInfo fenceInfo = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };

  if (vkCreateSemaphore(device, &semaphoreInfo, nullptr,
                        &m_image_available_sem) != VK_SUCCESS ||
      vkCreateSemaphore(device, &semaphoreInfo, nullptr,
                        &m_render_finished_sem) != VK_SUCCESS ||
      vkCreateFence(device, &fenceInfo, nullptr, &m_in_flight_fence) !=
          VK_SUCCESS) {
    XEV_ERROR("Failed to create synchronization objects for a frame!");
  }
}

Renderer::~Renderer() {
  VkDevice device = m_backend->get_device();
  vkDeviceWaitIdle(device);

  vkDestroySemaphore(device, m_render_finished_sem, nullptr);
  vkDestroySemaphore(device, m_image_available_sem, nullptr);
  vkDestroyFence(device, m_in_flight_fence, nullptr);

  vkDestroyCommandPool(device, m_cmd_pool, nullptr);

  if (m_vertex_buffer != VK_NULL_HANDLE) {
    vkDestroyBuffer(device, m_vertex_buffer, nullptr);
    vkFreeMemory(device, m_vertex_buffer_memory, nullptr);
  }
  if (m_index_buffer != VK_NULL_HANDLE) {
    vkDestroyBuffer(device, m_index_buffer, nullptr);
    vkFreeMemory(device, m_index_buffer_memory, nullptr);
  }

  vkDestroyPipeline(device, m_pipeline, nullptr);
  vkDestroyPipelineLayout(device, m_pipeline_layout, nullptr);
  vkDestroyDescriptorSetLayout(device, m_cam_ds_layout, nullptr);
}

uint32_t Renderer::find_memory_type(uint32_t typeFilter,
                                    VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceProperties prop;
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(m_backend->get_physical_device(),
                                      &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags &
                                    properties) == properties) {
      return i;
    }
  }

  XEV_ERROR("Failed to find suitable memory type!");
  return 0;
}

void Renderer::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
                             VkMemoryPropertyFlags properties, VkBuffer &buffer,
                             VkDeviceMemory &bufferMemory) {
  VkDevice device = m_backend->get_device();

  VkBufferCreateInfo bufferInfo = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = size,
      .usage = usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };

  if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
    XEV_ERROR("Failed to create buffer!");
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memRequirements.size,
      .memoryTypeIndex =
          find_memory_type(memRequirements.memoryTypeBits, properties),
  };

  if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) !=
      VK_SUCCESS) {
    XEV_ERROR("Failed to allocate buffer memory!");
  }

  vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void Renderer::create_pipeline(VkDevice device, VkFormat swapchain_format) {
  VkResult res_;

  if (device == VK_NULL_HANDLE) {
    XEV_ERROR("failed to create pipeline: invalid device");
    return;
  }

  // 1. camera data binding
  VkDescriptorSetLayoutBinding cam_layout_bind = {
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
      .pImmutableSamplers = nullptr,
  };

  VkDescriptorSetLayoutCreateInfo cam_layout_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = 1,
      .pBindings = &cam_layout_bind,
  };

  res_ = vkCreateDescriptorSetLayout(device, &cam_layout_info, nullptr,
                                     &m_cam_ds_layout);

  if (res_ != VK_SUCCESS) {
    XEV_ERROR("failed to create pipeline: cam ds layout creation failed");
    return;
  }

  // 2. pipeline layout
  VkPushConstantRange push_constant_range = {
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
      .offset = 0,
      .size = sizeof(glm::mat4),
  };

  VkPipelineLayoutCreateInfo pipe_layout_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &m_cam_ds_layout,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &push_constant_range,
  };

  res_ = vkCreatePipelineLayout(device, &pipe_layout_info, nullptr,
                                &m_pipeline_layout);

  if (res_ != VK_SUCCESS) {
    XEV_ERROR("failed to create pipeline: pipeline layout creation failed");
    return;
  }

  // 3. pipeline setup
  VkVertexInputBindingDescription bind_desc = {
      .binding = 0,
      .stride = sizeof(glm::vec3),
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
  };

  VkVertexInputAttributeDescription attr_desc = {
      .binding = 0,
      .location = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = 0,
  };

  VkPipelineVertexInputStateCreateInfo input_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &bind_desc,
      .vertexAttributeDescriptionCount = 1,
      .pVertexAttributeDescriptions = &attr_desc,
  };

  VkPipelineRenderingCreateInfo render_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &swapchain_format,
  };

  VkPipelineInputAssemblyStateCreateInfo input_assembly = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE,
  };

  VkPipelineViewportStateCreateInfo viewport_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .scissorCount = 1,
  };

  VkPipelineRasterizationStateCreateInfo rast_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_BACK_BIT,
      .frontFace = VK_FRONT_FACE_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,
      .lineWidth = 1.0f,
  };

  VkPipelineMultisampleStateCreateInfo msaa_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_4_BIT,
      .sampleShadingEnable = VK_FALSE,
  };

  VkPipelineColorBlendAttachmentState blend_att = {
      .blendEnable = VK_FALSE,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
  };

  VkPipelineColorBlendStateCreateInfo blend_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .attachmentCount = 1,
      .pAttachments = &blend_att,
  };

  std::vector<VkDynamicState> dyn_states = {VK_DYNAMIC_STATE_VIEWPORT,
                                            VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dyn_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = static_cast<uint32_t>(dyn_states.size()),
      .pDynamicStates = dyn_states.data(),
  };

  auto shader_stages = m_shader->get_stages();

  VkGraphicsPipelineCreateInfo pipe_info = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext = &render_info,
      .stageCount = static_cast<uint32_t>(shader_stages.size()),
      .pStages = shader_stages.data(),
      .pVertexInputState = &input_info,
      .pInputAssemblyState = &input_assembly,
      .pViewportState = &viewport_info,
      .pRasterizationState = &rast_info,
      .pMultisampleState = &msaa_info,
      .pColorBlendState = &blend_info,
      .pDynamicState = &dyn_info,
      .layout = m_pipeline_layout,
      .renderPass = VK_NULL_HANDLE,
  };

  res_ = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipe_info,
                                   nullptr, &m_pipeline);

  if (res_ != VK_SUCCESS) {
    XEV_ERROR("failed to create pipeline: graphics pipeline creation failed");
    return;
  }
}

void Renderer::recreate_pipeline(VkDevice device) {}

void Renderer::draw_triangle() {
  VkDevice device = m_backend->get_device();
  VkSwapchainKHR swapchain = m_backend->get_swapchain();

  if (vkWaitForFences(device, 1, &m_in_flight_fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS) {
    return;
  }
  vkResetFences(device, 1, &m_in_flight_fence);

  uint32_t image_index;
  VkResult result =
      vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
                            m_image_available_sem, VK_NULL_HANDLE, &image_index);

  if (result != VK_SUCCESS) {
    XEV_ERROR("draw_triangle: vkAcquireNextImageKHR failed with {}", (int)result);
    return;
  }

  XEV_INFO("Acquired image index: {}", image_index);

  vkResetCommandBuffer(m_cmd_buffer, 0);

  VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };

  if (vkBeginCommandBuffer(m_cmd_buffer, &beginInfo) != VK_SUCCESS) {
    return;
  }

  VkExtent2D extent = m_backend->get_extent();
  auto image_views = m_backend->get_swapchain_image_views();

  // Clear to a very obvious color (Magenta) to see if clear works
  VkRenderingAttachmentInfo colorAttachment = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = image_views[image_index],
      .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue = {.color = {.float32 = {1.0f, 0.0f, 1.0f, 1.0f}}},
  };

  VkRenderingInfo renderInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea = {.offset = {0, 0}, .extent = extent},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachment,
  };

  VkImageMemoryBarrier2 imageBarrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = 0,
      .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .image = m_backend->get_swapchain_images()[image_index],
      .subresourceRange =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };

  VkDependencyInfo depInfo = {
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &imageBarrier,
  };

  vkCmdPipelineBarrier2(m_cmd_buffer, &depInfo);
  vkCmdBeginRendering(m_cmd_buffer, &renderInfo);
  vkCmdBindPipeline(m_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

  glm::mat4 identity = glm::mat4(1.0f);
  vkCmdPushConstants(m_cmd_buffer, m_pipeline_layout,
                     VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &identity);

  VkViewport viewport = {0.0f, 0.0f, (float)extent.width, (float)extent.height, 0.0f, 1.0f};
  vkCmdSetViewport(m_cmd_buffer, 0, 1, &viewport);
  VkRect2D scissor = {{0, 0}, extent};
  vkCmdSetScissor(m_cmd_buffer, 0, 1, &scissor);

  if (m_vertex_buffer != VK_NULL_HANDLE) {
    VkBuffer vertexBuffers[] = {m_vertex_buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(m_cmd_buffer, 0, 1, vertexBuffers, offsets);
    vkCmdDraw(m_cmd_buffer, 3, 1, 0, 0);
  }

  vkCmdEndRendering(m_cmd_buffer);

  imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
  imageBarrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
  imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
  imageBarrier.dstAccessMask = 0;
  imageBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  imageBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  vkCmdPipelineBarrier2(m_cmd_buffer, &depInfo);

  if (vkEndCommandBuffer(m_cmd_buffer) != VK_SUCCESS) {
      return;
  }

  VkSemaphore waitSemaphores[] = {m_image_available_sem};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  VkSemaphore signalSemaphores[] = {m_render_finished_sem};

  VkSubmitInfo submitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = waitSemaphores,
      .pWaitDstStageMask = waitStages,
      .commandBufferCount = 1,
      .pCommandBuffers = &m_cmd_buffer,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = signalSemaphores,
  };

  if (vkQueueSubmit(m_backend->retrieve_queue(Q_GRAPHICS), 1, &submitInfo, m_in_flight_fence) != VK_SUCCESS) {
      return;
  }

  VkPresentInfoKHR presentInfo = {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = signalSemaphores,
      .swapchainCount = 1,
      .pSwapchains = &swapchain,
      .pImageIndices = &image_index,
  };
  vkQueuePresentKHR(m_backend->retrieve_queue(Q_PRESENT), &presentInfo);
}

void Renderer::draw() {
  VkDevice device = m_backend->get_device();
  VkSwapchainKHR swapchain = m_backend->get_swapchain();

  if (vkWaitForFences(device, 1, &m_in_flight_fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS) {
    return;
  }
  vkResetFences(device, 1, &m_in_flight_fence);

  uint32_t image_index;
  VkResult result =
      vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
                            m_image_available_sem, VK_NULL_HANDLE, &image_index);

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    XEV_ERROR("Failed to acquire swapchain image!");
    return;
  }

  vkResetCommandBuffer(m_cmd_buffer, 0);

  VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };

  if (vkBeginCommandBuffer(m_cmd_buffer, &beginInfo) != VK_SUCCESS) {
    XEV_ERROR("Failed to begin recording command buffer!");
    return;
  }

  VkExtent2D extent = m_backend->get_extent();
  auto image_views = m_backend->get_swapchain_image_views();

  VkRenderingAttachmentInfo colorAttachment = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .imageView = image_views[image_index],
      .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue = {.color = {.float32 = {0.1f, 0.1f, 0.1f, 1.0f}}},
  };

  VkRenderingInfo renderInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .renderArea =
          {
              .offset = {0, 0},
              .extent = extent,
          },
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachment,
  };

  VkImageMemoryBarrier2 imageBarrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = 0,
      .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .image = m_backend->get_swapchain_images()[image_index],
      .subresourceRange =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };

  VkDependencyInfo depInfo = {
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &imageBarrier,
  };

  vkCmdPipelineBarrier2(m_cmd_buffer, &depInfo);

  vkCmdBeginRendering(m_cmd_buffer, &renderInfo);

  vkCmdBindPipeline(m_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

  glm::mat4 vp_mat = m_scene->m_active_cam.create_vp_mat();

  vkCmdPushConstants(m_cmd_buffer, m_pipeline_layout,
                     VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &vp_mat);

  VkViewport viewport = {
      .x = 0.0f,
      .y = 0.0f,
      .width = (float)extent.width,
      .height = (float)extent.height,
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };
  vkCmdSetViewport(m_cmd_buffer, 0, 1, &viewport);

  VkRect2D scissor = {
      .offset = {0, 0},
      .extent = extent,
  };
  vkCmdSetScissor(m_cmd_buffer, 0, 1, &scissor);

  if (m_vertex_buffer != VK_NULL_HANDLE) {
    VkBuffer vertexBuffers[] = {m_vertex_buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(m_cmd_buffer, 0, 1, vertexBuffers, offsets);

    if (m_index_buffer != VK_NULL_HANDLE) {
      vkCmdBindIndexBuffer(m_cmd_buffer, m_index_buffer, 0,
                           VK_INDEX_TYPE_UINT32);
      vkCmdDrawIndexed(m_cmd_buffer, m_index_count, 1, 0, 0, 0);
    } else {
      vkCmdDraw(m_cmd_buffer, static_cast<uint32_t>(m_scene->m_vert_buffer.size()), 1, 0, 0);
    }
  }

  vkCmdEndRendering(m_cmd_buffer);

  imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
  imageBarrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
  imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
  imageBarrier.dstAccessMask = 0;
  imageBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  imageBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  vkCmdPipelineBarrier2(m_cmd_buffer, &depInfo);

  if (vkEndCommandBuffer(m_cmd_buffer) != VK_SUCCESS) {
    XEV_ERROR("Failed to record command buffer!");
    return;
  }

  VkSemaphore waitSemaphores[] = {m_image_available_sem};
  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  VkSemaphore signalSemaphores[] = {m_render_finished_sem};

  VkSubmitInfo submitInfo = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = waitSemaphores,
      .pWaitDstStageMask = waitStages,
      .commandBufferCount = 1,
      .pCommandBuffers = &m_cmd_buffer,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = signalSemaphores,
  };

  if (vkQueueSubmit(m_backend->retrieve_queue(Q_GRAPHICS), 1, &submitInfo,
                    m_in_flight_fence) != VK_SUCCESS) {
    XEV_ERROR("Failed to submit draw command buffer!");
    return;
  }

  VkSwapchainKHR swapchains[] = {swapchain};

  VkPresentInfoKHR presentInfo = {
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = signalSemaphores,
      .swapchainCount = 1,
      .pSwapchains = swapchains,
      .pImageIndices = &image_index,
  };

  vkQueuePresentKHR(m_backend->retrieve_queue(Q_PRESENT), &presentInfo);
}

} // namespace xev
