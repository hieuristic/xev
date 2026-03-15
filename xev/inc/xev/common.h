#include <xev/volk.h>

namespace xev {
namespace common {
constexpr VkImageSubresourceRange image_subresource_range(
    VkImageAspectFlagBits aspect_mask) {
  return VkImageSubresourceRange{
      .aspectMask = aspect_mask,
      .baseMipLevel = 0,
      .levelCount = VK_REMAINING_MIP_LEVELS,
      .baseArrayLayer = 0,
      .layerCount = VK_REMAINING_ARRAY_LAYERS,
  };
}

VkImageLayout update_layout(VkCommandBuffer cmd,
                            VkImage image,
                            VkImageLayout old_layout,
                            VkImageLayout new_layout) {
  VkImageAspectFlags aspect_mask =
      (old_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
       new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
       new_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL)
          ? VK_IMAGE_ASPECT_DEPTH_BIT
          : VK_IMAGE_ASPECT_COLOR_BIT;
  VkImageMemoryBarrier2 image_barrier{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
      .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
      .dstAccessMask =
          VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
      .oldLayout = old_layout,
      .newLayout = new_layout,
      .image = image,
      .subresourceRange = image_subresource_range(aspect_mask),
  };

  VkDependencyInfo dep_info{
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &image_barrier,
  };

  vkCmdPipelineBarrier2(cmd, &dep_info);
  return new_layout;
}

void copy_image(VkCommandBuffer cmd,
                VkImage src,
                VkImage dst,
                VkExtend2D src_ext,
                VkExtend2D dst_ext,
                VkFilter) {
  VkBlitImageInfo2 blit_info = {
      .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
      .srcImage = src,
      .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      .dstImage = dst,
      .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      .regionCount = 1,
      .pRegions =
          &{
              .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
              .srcSubresource =
                  {
                      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                      .mipLevel = 0,
                      .baseArrayLayer = 0,
                      .layerCount = 1,
                  },
              .srcOffsets =
                  {
                      {},
                      {(std::int32_t)src_ext.width,
                       (std::int32_t)src_ext.height, 1},
                  },
              .dstSubresource =
                  {
                      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                      .mipLevel = 0,
                      .baseArrayLayer = 0,
                      .layerCount = 1,
                  },
              .dstOffsets =
                  {
                      {},
                      {(std::int32_t)dst_ext.width,
                       (std::int32_t)dst_ext.height, 1},
                  },
          },
      .filter = filter,
  };

  vkCmdBlitImage2(cmd, &blit_info);
}

}  // namespace common
}  // namespace xev
