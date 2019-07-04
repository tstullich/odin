#ifndef ODIN_DEVICE_MANAGER_HPP
#define ODIN_DEVICE_MANAGER_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "vk/instance.hpp"

namespace odin {

// TODO See if we can optimize queue selection according to this post under
// 'Transfer Queue' https://vulkan-tutorial.com/Vertex_buffers/Staging_buffer
struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool isComplete() {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

class DeviceManager {
 public:
  DeviceManager(const Instance &instance, const VkSurfaceKHR &surface,
                bool enableValidationLayers);

  ~DeviceManager();

  QueueFamilyIndices findQueueFamilies(VkSurfaceKHR surface);

  const VkDevice getLogicalDevice() const;

  const VkQueue getGraphicsQueue() const;

  const VkPhysicalDevice getPhysicalDevice() const;

  const VkQueue getPresentationQueue() const;

  SwapChainSupportDetails getSwapChainSupport() const;

 private:
  bool checkDeviceExtensionSupport(VkPhysicalDevice device);

  void createLogicalDevice(const Instance &instance, VkSurfaceKHR surface,
                           bool enableValidationLayers);

  bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);

  void pickPhysicalDevice(const Instance &instance, VkSurfaceKHR surface);

  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice,
                                       VkSurfaceKHR surface);

  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice,
                                                VkSurfaceKHR surface);

  const std::vector<const char *> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  SwapChainSupportDetails swapChainSupportDetails;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkDevice logicalDevice;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
};
}  // namespace odin
#endif  // ODIN_DEVICE_MANAGER_HPP