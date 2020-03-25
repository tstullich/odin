#ifndef ODIN_SWAPCHAIN_HPP
#define ODIN_SWAPCHAIN_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <array>
#include <stdexcept>
#include <vector>

#include "vk/device_manager.hpp"
#include "vk/render_pass.hpp"

namespace odin {
class Swapchain {
 public:
  Swapchain(const odin::QueueFamilyIndices& queueFamiles,
            const odin::SwapChainSupportDetails& details,
            const VkDevice& logicalDevice, const VkSurfaceKHR& surface,
            GLFWwindow* window);

  void cleanup();

  VkImageView createImageView(const VkDevice& logicalDevice,
                              const VkImage& image, const VkFormat& format,
                              const VkImageAspectFlags& aspectMask) const;

  void createFrameBuffers(const VkDevice& logicalDevice,
                          const RenderPass& renderPass,
                          const VkImageView& depthImageView);

  void createImageViews(const VkDevice& logicalDevice);

  VkExtent2D getExtent() const;

  VkFramebuffer getFrameBuffer(size_t index) const;

  std::vector<VkFramebuffer> getFramebuffers() const;

  size_t getFrameBufferSizes() const;

  VkFormat getImageFormat() const;

  size_t getImageSize() const;

  std::vector<VkImageView> getImageViews() const;

  VkSwapchainKHR getSwapchain() const;

  void recreateSwapChain();

 private:
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
                              GLFWwindow* window);

  VkPresentModeKHR choosePresentMode(
      const std::vector<VkPresentModeKHR>& availablePresentModes);

  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR>& availableFormats);

  void createSwapChain(const odin::QueueFamilyIndices& queueFamilies,
                       const odin::SwapChainSupportDetails& details,
                       const VkDevice& device, const VkSurfaceKHR& surface,
                       GLFWwindow* window);

  VkSwapchainKHR swapChain;
  std::vector<VkImage> swapChainImages;
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;
  std::vector<VkImageView> swapChainImageViews;
  std::vector<VkFramebuffer> swapChainFramebuffers;
};
}  // namespace odin
#endif  // ODIN_SWAPCHAIN_HPP