#include <xev/common.h>
#include <xev/logger.h>
#include <xev/resource/image.h>
#include <xev/resource/buffer.h>
#include <xev/resource_manager.h>

namespace xev {

void Image::set_layout(const VkImageLayout& layout_) {
  XEV_ASSERT(layout == VK_IMAGE_LAYOUT_UNDEFINED);
  layout = layout_;
}

void Image::update_layout(const VkCommandBuffer& cmdbuf,
                          const VkImageLayout& new_layout) {
  update_layout(cmdbuf, layout, new_layout);
  layout = new_layout;
}

void Image::update_layout(const VkCommandBuffer& cmdbuf,
                          const VkImageLayout& old_layout,
                          const VkImageLayout& new_layout) {
  VkImageAspectFlags aspect_mask =
      (layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
       new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
       new_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL)
          ? VK_IMAGE_ASPECT_DEPTH_BIT
          : VK_IMAGE_ASPECT_COLOR_BIT;

  VkImageMemoryBarrier2 barrier{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .oldLayout = old_layout,
      .newLayout = new_layout,
      .image = image,
      .subresourceRange =
          {
              .aspectMask = aspect_mask,
              .baseMipLevel = 0,
              .levelCount = VK_REMAINING_MIP_LEVELS,
              .baseArrayLayer = 0,
              .layerCount = VK_REMAINING_ARRAY_LAYERS,
          },
  };

  if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
      new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
    barrier.srcAccessMask = VK_ACCESS_2_NONE;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
  } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
  } else {
    // Fallback for everything else
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.dstAccessMask =
        VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
  }

  VkDependencyInfo dep_info{
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers = &barrier,
  };

  vkCmdPipelineBarrier2(cmdbuf, &dep_info);
}

void Image::blit_from(const VkCommandBuffer& cmdbuf,
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

  vkCmdBlitImage2(cmdbuf, &blit_info);
}

void Image::upload(const ResourceManager& manager, const HotExec& hot_exec) {
  XEV_ASSERT(host_data.size() != 0, "Trying to upload empty image!");

  Buffer staging{
      width * height * 4,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VMA_MEMORY_USAGE_AUTO,
  };
  manager.alloc(staging);

  void* map_ = staging.alloc_info.pMappedData;
  memcpy(map_, host_data.data(), host_data.size());

  hot_exec.run([&](const VkCommandBuffer cmdbuf) {
    this->layout = VK_IMAGE_LAYOUT_UNDEFINED;
    this->update_layout(cmdbuf, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkBufferImageCopy reg = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        .imageOffset = {0, 0, 0},
        .imageExtent =
            {
                static_cast<uint32_t>(this->width),
                static_cast<uint32_t>(this->height),
                1,
            },
    };
    vkCmdCopyBufferToImage(cmdbuf, staging.buffer, this->image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &reg);

    this->update_layout(cmdbuf, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  });
  manager.free(staging);
}

}  // namespace xev
