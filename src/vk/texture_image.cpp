#include "vk/texture_image.hpp"

odin::TextureImage::TextureImage(const DeviceManager& deviceManager,
                                 const CommandPool& commandPool,
                                 const Swapchain& swapChain,
                                 const TextureSampler& textureSampler,
                                 uint32_t width, uint32_t height) {
  createTextureImage(deviceManager, commandPool, width, height);
  createTextureImageView(deviceManager, swapChain);

  descriptor.sampler = textureSampler.getSampler();
  descriptor.imageView = textureImageView;
  descriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
}

void odin::TextureImage::copyBufferToImage(const DeviceManager& deviceManager,
                                           const CommandPool& commandPool,
                                           VkBuffer buffer, VkImage image,
                                           uint32_t width, uint32_t height) {
  VkCommandBuffer commandBuffer =
      commandPool.beginSingleTimeCommands(deviceManager.getLogicalDevice());

  VkBufferImageCopy region = {};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;

  region.imageOffset = {0, 0, 0};
  region.imageExtent = {width, height, 1};

  vkCmdCopyBufferToImage(commandBuffer, buffer, image,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  commandPool.endSingleTimeCommands(deviceManager, commandBuffer);
}

void odin::TextureImage::createTextureImage(const DeviceManager& deviceManager,
                                            const CommandPool& commandPool,
                                            uint32_t width, uint32_t height) {
  imageHeight = height;
  imageWidth = width;

  // Get device properties for the requested texture format
  // TODO Check if we could use a better image format
  VkFormatProperties formatProperties;
  vkGetPhysicalDeviceFormatProperties(deviceManager.getPhysicalDevice(),
                                      VK_FORMAT_R8G8B8A8_UNORM,
                                      &formatProperties);

  // Check if requested image format supports image storage operations
  if (!(formatProperties.optimalTilingFeatures &
        VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
    throw std::runtime_error(
        "Could not find supported image format for compute texture!");
  }

  // Create a texture that is used for storage in the compute shader
  // and can be sampled from in the fragment shader
  createImage(deviceManager, width, height, VK_FORMAT_R8G8B8A8_UNORM,
              VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, textureImageMemory);

  // Setup the image layout for the texture
  transitionImageLayout(deviceManager, commandPool, image,
                        VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT,
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
}

void odin::TextureImage::createTextureImageView(
    const DeviceManager& deviceManager, const Swapchain& swapChain) {
  textureImageView = swapChain.createImageView(deviceManager.getLogicalDevice(),
                                               image, VK_FORMAT_R8G8B8A8_UNORM,
                                               VK_IMAGE_ASPECT_COLOR_BIT);
}

const VkDescriptorImageInfo* odin::TextureImage::getDescriptor() const {
  return &descriptor;
}

const uint32_t odin::TextureImage::getHeight() const { return imageHeight; }

const VkImage odin::TextureImage::getTextureImage() const { return image; }

const VkDeviceMemory odin::TextureImage::getTextureImageMemory() const {
  return textureImageMemory;
}

const VkImageView odin::TextureImage::getTextureImageView() const {
  return textureImageView;
}

const uint32_t odin::TextureImage::getWidth() const { return imageWidth; }