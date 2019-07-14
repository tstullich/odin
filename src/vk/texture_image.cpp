#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "vk/texture_image.hpp"

odin::TextureImage::TextureImage(const DeviceManager& deviceManager,
                                 const CommandPool& commandPool,
                                 const Swapchain& swapChain,
                                 const std::string& texturePath) {
  createTextureImage(deviceManager, commandPool, texturePath);
  createTextureImageView(deviceManager, swapChain);
}

odin::TextureImage::~TextureImage() {
  std::cout << "IMPLEMENT TEXTURE IMAGE DESTRUCTOR!" << std::endl;
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
                                            const std::string& texturePath) {
  int texWidth, texHeight, texChannels;
  stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight,
                              &texChannels, STBI_rgb_alpha);
  VkDeviceSize imageSize = texWidth * texHeight * 4;
  mipLevels = static_cast<uint32_t>(
                  std::floor(std::log2(std::max(texWidth, texHeight)))) +
              1;

  if (!pixels) {
    throw std::runtime_error("Failed to load texture image!");
  }

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  Buffer::createBuffer(deviceManager, imageSize,
                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       stagingBuffer, stagingBufferMemory);

  void* data;
  vkMapMemory(deviceManager.getLogicalDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
  memcpy(data, pixels, static_cast<size_t>(imageSize));
  vkUnmapMemory(deviceManager.getLogicalDevice(), stagingBufferMemory);

  stbi_image_free(pixels);

  createImage(deviceManager, texWidth, texHeight, mipLevels,
              VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, textureImageMemory);

  transitionImageLayout(deviceManager, commandPool, image,
                        VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);

  copyBufferToImage(deviceManager, commandPool, stagingBuffer, image,
                    static_cast<uint32_t>(texWidth),
                    static_cast<uint32_t>(texHeight));

  // Create a mipmap from the loaded texture
  generateMipmaps(deviceManager, commandPool, image, VK_FORMAT_R8G8B8A8_UNORM,
                  texWidth, texHeight, mipLevels);
}

void odin::TextureImage::createTextureImageView(
    const DeviceManager& deviceManager, const Swapchain& swapChain) {
  textureImageView = swapChain.createImageView(
      deviceManager.getLogicalDevice(), image, VK_FORMAT_R8G8B8A8_UNORM,
      VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
}

const uint32_t odin::TextureImage::getMipLevels() const { return mipLevels; }

const VkImage odin::TextureImage::getTextureImage() const { return image; }

const VkDeviceMemory odin::TextureImage::getTextureImageMemory() const {
  return textureImageMemory;
}

const VkImageView odin::TextureImage::getTextureImageView() const {
  return textureImageView;
}

void odin::TextureImage::generateMipmaps(const DeviceManager& deviceManager,
                                         const CommandPool& commandPool,
                                         VkImage image, VkFormat imageFormat,
                                         int32_t texWidth, int32_t texHeight,
                                         uint32_t mipLevels) {
  // Check if image format supports linear blitting
  VkFormatProperties formatProperties;
  vkGetPhysicalDeviceFormatProperties(deviceManager.getPhysicalDevice(),
                                      imageFormat, &formatProperties);

  if (!(formatProperties.optimalTilingFeatures &
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
    throw std::runtime_error(
        "Texture image format does not support linear blitting!");
  }

  VkCommandBuffer commandBuffer =
      commandPool.beginSingleTimeCommands(deviceManager.getLogicalDevice());

  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.image = image;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.subresourceRange.levelCount = 1;

  int32_t mipWidth = texWidth;
  int32_t mipHeight = texHeight;

  for (uint32_t i = 1; i < mipLevels; i++) {
    barrier.subresourceRange.baseMipLevel = i - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                         nullptr, 1, &barrier);

    VkImageBlit blit = {};
    blit.srcOffsets[0] = {0, 0, 0};
    blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
    blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.srcSubresource.mipLevel = i - 1;
    blit.srcSubresource.baseArrayLayer = 0;
    blit.srcSubresource.layerCount = 1;
    blit.dstOffsets[0] = {0, 0, 0};
    blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1,
                          mipHeight > 1 ? mipHeight / 2 : 1, 1};
    blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.dstSubresource.mipLevel = i;
    blit.dstSubresource.baseArrayLayer = 0;
    blit.dstSubresource.layerCount = 1;

    vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
                   VK_FILTER_LINEAR);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr,
                         0, nullptr, 1, &barrier);

    if (mipWidth > 1) {
      mipWidth /= 2;
    }
    if (mipHeight > 1) {
      mipHeight /= 2;
    }
  }

  // Transition last mipmap level into memory
  barrier.subresourceRange.baseMipLevel = mipLevels - 1;
  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0,
                       nullptr, 1, &barrier);

  commandPool.endSingleTimeCommands(deviceManager, commandBuffer);
}