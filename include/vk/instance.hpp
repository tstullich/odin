#ifndef ODIN_INSTANCE_HPP
#define ODIN_INSTANCE_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

/**
 * Class that wraps a VkInstance and
 * provides access to various functionality to manage it
 */
namespace odin {
class Instance {
 public:
  Instance(bool enableValidationLayers);

  ~Instance();

  const VkDebugUtilsMessengerEXT getDebugMessenger() const;

  const VkInstance getInstance() const;

  const std::vector<const char *> getValidationLayerData() const;

  size_t getValidationLayerSize() const;

 private:
  bool checkValidationLayerSupport();

  VkResult createDebugUtilsMessengerEXT(
      VkInstance instance,
      const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
      const VkAllocationCallbacks *pAllocator,
      VkDebugUtilsMessengerEXT *pDebugMessenger);

  void destroyDebugUtilsMessengerEXT(VkInstance instance,
                                     VkDebugUtilsMessengerEXT debugMessenger,
                                     const VkAllocationCallbacks *pAllocator);

  std::vector<const char *> getRequiredExtensions();

  void populateDebugMessengerCreateInfo(
      VkDebugUtilsMessengerCreateInfoEXT &createInfo);

  void setupDebugMessenger();

  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                void *pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
  }

  const std::vector<const char *> validationLayers = {
      "VK_LAYER_KHRONOS_validation"};

  bool enableValidationLayers;

  VkDebugUtilsMessengerEXT debugMessenger;
  VkInstance instance;
};
}  // namespace odin
#endif  // ODIN_INSTANCE_HPP