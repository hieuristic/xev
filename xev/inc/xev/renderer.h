#pragma once
#include <xev/volk.h>

namespace xev {

class Image;

class Renderer {
 protected:
  void prepare_attachments(VkCommandBuffer& cmdbuf,
                           const Image& color_image);
  void prepare_attachments(VkCommandBuffer& cmdbuf,
                           const Image& color_image,
                           const Image& depth_image);
  void prepare_transfer(VkCommandBuffer& cmdbuf, const Image& color_image);
};

}  // namespace xev
