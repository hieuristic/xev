#pragma once
#include <xev/volk.h>

namespace xev {

struct Image;

struct Renderer {
 protected:
  void prepare_attachments(VkCommandBuffer& cmdbuf,
                           const Image& color_image);
  void prepare_attachments(VkCommandBuffer& cmdbuf,
                           const Image& color_image,
                           const Image& depth_image);
  void prepare_transfer(VkCommandBuffer& cmdbuf, const Image& color_image);
};

}  // namespace xev
