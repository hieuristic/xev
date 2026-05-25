#include <xev/common.h>
#include <xev/resource/image.h>
#include <xev/resource_manager.h>

namespace xev {

void Image::set_layout(const VkImageLayout& layout_) {
  XEV_ASSERT(layout == VK_IMAGE_LAYOUT_UNDEFINED);
  layout = layout_;
}

void Image::update_layout(const VkCommandBuffer& cmd,
                          const VkImageLayout& new_layout) {
  VkImageAspectFlags aspect_mask =
      (layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
       new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
       new_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL)
          ? VK_IMAGE_ASPECT_DEPTH_BIT
          : VK_IMAGE_ASPECT_COLOR_BIT;

  VkImageMemoryBarrier2 barrier{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
      .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
      .dstAccessMask =
          VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
      .oldLayout = layout,
      .newLayout = new_layout,
      .image = image,
      .subresourceRange = common::image_subresource_range(aspect_mask),
  };

  VkDependencyInfo dep_info{
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &barrier,
  };

  vkCmdPipelineBarrier2(cmd, &dep_info);
}

void Image::blit_to(const VkCommandBuffer& cmd,
                    const Image& src,
                    const VkFilter& filter) {
  VkImageBlit2 region = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
      .srcSubresource =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .mipLevel = 0,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
      .srcOffsets = {{},
                     {static_cast<int32_t>(src.width),
                      static_cast<int32_t>(src.height), 1}},
      .dstSubresource =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .mipLevel = 0,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
      .dstOffsets = {{},
                     {static_cast<int32_t>(width), static_cast<int32_t>(height),
                      1}},
  };

  VkBlitImageInfo2 blit_info = {
      .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
      .srcImage = src.image,
      .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      .dstImage = image,
      .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      .regionCount = 1,
      .pRegions = &region,
      .filter = filter,
  };

  vkCmdBlitImage2(cmd, &blit_info);
}

}  // namespace xev
