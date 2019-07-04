#include <vk/device_manager.hpp>

odin::DeviceManager::DeviceManager(const Instance& instance,
                                   const VkSurfaceKHR& surface,
                                   bool enableValidationLayers) {
  pickPhysicalDevice(instance, surface);
  createLogicalDevice(instance, surface, enableValidationLayers);
}

odin::DeviceManager::~DeviceManager() {
  // TODO Move destructor logic out of main.cpp
  std::cout << "IMPLEMENT DEVICE MANAGER DESTRUCTOR!" << std::endl;
}

bool odin::DeviceManager::checkDeviceExtensionSupport(
    VkPhysicalDevice physicalDevice) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount,
                                       nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount,
                                       availableExtensions.data());

  std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                           deviceExtensions.end());

  for (const auto& extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

void odin::DeviceManager::createLogicalDevice(const Instance& instance,
                                              VkSurfaceKHR surface,
                                              bool enableValidationLayers) {
  QueueFamilyIndices indices = findQueueFamilies(surface);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
                                            indices.presentFamily.value()};

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures = {};
  // Need to check for this even though device should have it
  deviceFeatures.samplerAnisotropy = VK_TRUE;

  VkDeviceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  createInfo.queueCreateInfoCount =
      static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos = queueCreateInfos.data();

  createInfo.pEnabledFeatures = &deviceFeatures;

  createInfo.enabledExtensionCount =
      static_cast<uint32_t>(deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();

  if (enableValidationLayers) {
    createInfo.enabledLayerCount =
        static_cast<uint32_t>(instance.getValidationLayerSize());
    createInfo.ppEnabledLayerNames = instance.getValidationLayerData().data();
  } else {
    createInfo.enabledLayerCount = 0;
  }

  if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create logical device!");
  }

  vkGetDeviceQueue(logicalDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
  vkGetDeviceQueue(logicalDevice, indices.presentFamily.value(), 0, &presentQueue);
}

// Helper function to allow access to queue family indices without having
// to specify the device. The device manager will use the currently selected
// device
odin::QueueFamilyIndices odin::DeviceManager::findQueueFamilies(
    VkSurfaceKHR surface) {
  return findQueueFamilies(physicalDevice, surface);
}

odin::QueueFamilyIndices odin::DeviceManager::findQueueFamilies(
    VkPhysicalDevice pDevice, VkSurfaceKHR surface) {
  // Check if physical device has been initialized
  if (pDevice == VK_NULL_HANDLE) {
    throw std::runtime_error("Physical device has not been initialized!");
  }

  odin::QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount,
                                           queueFamilies.data());

  int i = 0;
  for (const auto& queueFamily : queueFamilies) {
    if (queueFamily.queueCount > 0 &&
        queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
    }

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, i, surface, &presentSupport);

    if (queueFamily.queueCount > 0 && presentSupport) {
      indices.presentFamily = i;
    }

    if (indices.isComplete()) {
      break;
    }

    i++;
  }

  return indices;
}

const VkDevice odin::DeviceManager::getLogicalDevice() const { return logicalDevice; }

const VkQueue odin::DeviceManager::getGraphicsQueue() const {
  return graphicsQueue;
}

const VkPhysicalDevice odin::DeviceManager::getPhysicalDevice() const {
  return physicalDevice;
}

const VkQueue odin::DeviceManager::getPresentationQueue() const {
  return presentQueue;
}

odin::SwapChainSupportDetails odin::DeviceManager::getSwapChainSupport() const {
  // TODO Implement mechanism for proper caching in case devices are swapped for
  // an instance
  return swapChainSupportDetails;
}

bool odin::DeviceManager::isDeviceSuitable(VkPhysicalDevice physicalDevice,
                                           VkSurfaceKHR surface) {
  QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

  bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);

  bool swapChainAdequate = false;
  if (extensionsSupported) {
    SwapChainSupportDetails swapChainSupport =
        querySwapChainSupport(physicalDevice, surface);
    swapChainAdequate = !swapChainSupport.formats.empty() &&
                        !swapChainSupport.presentModes.empty();
  }

  VkPhysicalDeviceFeatures supportedFeatures;
  vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

  return indices.isComplete() && extensionsSupported && swapChainAdequate &&
         supportedFeatures.samplerAnisotropy;
}

void odin::DeviceManager::pickPhysicalDevice(const Instance& instance,
                                             VkSurfaceKHR surface) {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance.getInstance(), &deviceCount, nullptr);

  if (deviceCount == 0) {
    throw std::runtime_error("Failed to find GPUs with Vulkan support!");
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance.getInstance(), &deviceCount,
                             devices.data());

  for (const auto& pDevice : devices) {
    if (isDeviceSuitable(pDevice, surface)) {
      physicalDevice = pDevice;
      break;
    }
  }

  if (physicalDevice == VK_NULL_HANDLE) {
    throw std::runtime_error("Failed to find a suitable GPU!");
  }
}

odin::SwapChainSupportDetails odin::DeviceManager::querySwapChainSupport(
    VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
  if (physicalDevice == VK_NULL_HANDLE) {
    throw std::runtime_error(
        "Physical device has not been initialized. Cannot query swap chain "
        "support!");
  }

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      physicalDevice, surface, &swapChainSupportDetails.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount,
                                       nullptr);

  if (formatCount != 0) {
    swapChainSupportDetails.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice, surface, &formatCount,
        swapChainSupportDetails.formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,
                                            &presentModeCount, nullptr);

  if (presentModeCount != 0) {
    swapChainSupportDetails.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice, surface, &presentModeCount,
        swapChainSupportDetails.presentModes.data());
  }

  return swapChainSupportDetails;
}