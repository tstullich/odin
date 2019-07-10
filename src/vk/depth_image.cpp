#include "vk/depth_image.hpp"

odin::DepthImage::DepthImage(const DeviceManager& deviceManager,
                             const CommandPool& commandPool,
                             const Swapchain& swapChain) {
  createDepthResources(deviceManager, commandPool, swapChain);
}

odin::DepthImage::~DepthImage() {
  std::cout << "IMPLEMENT DEPTH IMAGE DESTRUCTOR!" << std::endl;
}

void odin::DepthImage::createDepthResources(const DeviceManager& deviceManager,
                                            const CommandPool& commandPool,
                                            const Swapchain& swapChain) {
  VkFormat depthFormat = findDepthFormat(deviceManager);

  auto swapChainExtent = swapChain.getExtent();
  createImage(deviceManager, swapChainExtent.width, swapChainExtent.height, 1,
              depthFormat, VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, depthImageMemory);

  depthImageView =
      swapChain.createImageView(deviceManager.getLogicalDevice(), image,
                                depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

  transitionImageLayout(deviceManager, commandPool, image, depthFormat,
                        VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}

VkFormat odin::DepthImage::findDepthFormat(const DeviceManager& deviceManager) {
  return findSupportedFormat(
      deviceManager,
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
       VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkFormat odin::DepthImage::findSupportedFormat(
    const DeviceManager& deviceManager, const std::vector<VkFormat>& candidates,
    VkImageTiling tiling, VkFormatFeatureFlags features) {
  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(deviceManager.getPhysicalDevice(),
                                        format, &props);

    // More formats are available but we only need linear and optimal tiling
    // for now
    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
               (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  throw std::runtime_error("Failed to find supported format!");
}
