#include <vk/swapchain.hpp>

odin::Swapchain::Swapchain(const odin::QueueFamilyIndices& queueFamilies,
                           const odin::SwapChainSupportDetails& details,
                           const VkDevice& logicalDevice,
                           const VkSurfaceKHR& surface, GLFWwindow* window) {
  createSwapChain(queueFamilies, details, logicalDevice, surface, window);
  createImageViews(logicalDevice);
}

odin::Swapchain::~Swapchain() {
  std::cout << "IMPLEMENT SWAPCHAIN DESTRUCTOR!" << std::endl;
}

void odin::Swapchain::cleanup() {
  // TODO Implement cleanup logic here
}

VkExtent2D odin::Swapchain::getExtent() const { return swapChainExtent; }

VkFramebuffer odin::Swapchain::getFrameBuffer(size_t index) const {
  return swapChainFramebuffers[index];
}

std::vector<VkFramebuffer> odin::Swapchain::getFramebuffers() const {
  return swapChainFramebuffers;
}

size_t odin::Swapchain::getFrameBufferSizes() const {
  return swapChainFramebuffers.size();
}

VkFormat odin::Swapchain::getImageFormat() const {
  return swapChainImageFormat;
}

size_t odin::Swapchain::getImageSize() const { return swapChainImages.size(); }

std::vector<VkImageView> odin::Swapchain::getImageViews() const {
  return swapChainImageViews;
}

VkSwapchainKHR odin::Swapchain::getSwapchain() const { return swapChain; }

VkExtent2D odin::Swapchain::chooseSwapExtent(
    const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
  // TODO Fix this from the tutorial 'Swap chain recreation'
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actualExtent = {static_cast<uint32_t>(width),
                               static_cast<uint32_t>(height)};

    actualExtent.width = std::max(
        capabilities.minImageExtent.width,
        std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(
        capabilities.minImageExtent.height,
        std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
  }
}

VkPresentModeKHR chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& availablePresentModes) {
  VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

  for (const auto& availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return availablePresentMode;
    } else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
      bestMode = availablePresentMode;
    }
  }

  return bestMode;
}

VkSurfaceFormatKHR odin::Swapchain::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats) {
  for (const auto& availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

void odin::Swapchain::createFrameBuffers(const VkDevice& logicalDevice,
                                         const RenderPass& renderPass,
                                         const VkImageView& depthImageView) {
  // Buffer size needs to match image views
  swapChainFramebuffers.resize(swapChainImageViews.size());

  for (size_t i = 0; i < swapChainImageViews.size(); i++) {
    std::array<VkImageView, 2> attachments = {swapChainImageViews[i],
                                              depthImageView};

    VkFramebufferCreateInfo frameBufferInfo = {};
    frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferInfo.renderPass = renderPass.getRenderPass();
    frameBufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    frameBufferInfo.pAttachments = attachments.data();
    frameBufferInfo.width = swapChainExtent.width;
    frameBufferInfo.height = swapChainExtent.height;
    frameBufferInfo.layers = 1;

    if (vkCreateFramebuffer(logicalDevice, &frameBufferInfo, nullptr,
                            &swapChainFramebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create framebuffer!");
    }
  }
}

VkImageView odin::Swapchain::createImageView(
    const VkDevice& logicalDevice, const VkImage& image, const VkFormat& format,
    const VkImageAspectFlags& aspectFlags, uint32_t mipLevels) const {
  VkImageViewCreateInfo viewInfo = {};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = aspectFlags;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = mipLevels;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  VkImageView imageView;
  if (vkCreateImageView(logicalDevice, &viewInfo, nullptr, &imageView) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create texture image view!");
  }

  return imageView;
}

void odin::Swapchain::createImageViews(const VkDevice& logicalDevice) {
  swapChainImageViews.resize(swapChainImages.size());

  for (size_t i = 0; i < swapChainImages.size(); i++) {
    swapChainImageViews[i] =
        createImageView(logicalDevice, swapChainImages[i], swapChainImageFormat,
                        VK_IMAGE_ASPECT_COLOR_BIT, 1);
  }
}

void odin::Swapchain::createSwapChain(
    const odin::QueueFamilyIndices& queueFamilies,
    const odin::SwapChainSupportDetails& details, const VkDevice& device,
    const VkSurfaceKHR& surface, GLFWwindow* window) {
  VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(details.formats);
  VkPresentModeKHR presentMode = chooseSwapPresentMode(details.presentModes);
  VkExtent2D extent = chooseSwapExtent(details.capabilities, window);

  uint32_t imageCount = details.capabilities.minImageCount + 1;
  if (details.capabilities.maxImageCount > 0 &&
      imageCount > details.capabilities.maxImageCount) {
    imageCount = details.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = surface;

  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  uint32_t queueFamilyIndices[] = {queueFamilies.graphicsFamily.value(),
                                   queueFamilies.presentFamily.value()};

  if (queueFamilies.graphicsFamily != queueFamilies.presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  createInfo.preTransform = details.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  createInfo.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create swap chain!");
  }

  vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
  swapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(device, swapChain, &imageCount,
                          swapChainImages.data());

  swapChainImageFormat = surfaceFormat.format;
  swapChainExtent = extent;
}