#include <xev/renderer.h>
#include <xev/resource/image.h>

namespace xev {

void Renderer::prepare_attachments(VkCommandBuffer& cmdbuf,
                                   const Image& color_image) {
  VkImageMemoryBarrier2 barrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
      .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .image = color_image.image,
      .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
  };

  VkDependencyInfo info = {
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &barrier,
  };
  vkCmdPipelineBarrier2(cmdbuf, &info);
}

void Renderer::prepare_attachments(VkCommandBuffer& cmdbuf,
                                   const Image& color_image,
                                   const Image& depth_image) {
  std::array<VkImageMemoryBarrier2, 2> barriers = {
      {{
           // Color barrier
           .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
           .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
           .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
           .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
           .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
           .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
           .image = color_image.image,
           .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
       },
       {
           // Depth barrier
           .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
           .srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                           VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
           .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                           VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
           .dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
           .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
           .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
           .image = depth_image.image,
           .subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1},
       }}};

  VkDependencyInfo info = {
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = (uint32_t)barriers.size(),
      .pImageMemoryBarriers = barriers.data(),
  };
  vkCmdPipelineBarrier2(cmdbuf, &info);
}

void Renderer::prepare_transfer(VkCommandBuffer& cmdbuf,
                                const Image& color_image) {
  VkImageMemoryBarrier2 barrier = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT,
      .dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
      .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      .image = color_image.image,
      .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
  };
  VkDependencyInfo info = {
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &barrier,
  };
  vkCmdPipelineBarrier2(cmdbuf, &info);
}

}  // namespace xev
