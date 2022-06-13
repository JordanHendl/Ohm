#pragma once
#include <vector>
#include <string>
#include <vulkan/vulkan.hpp>
#include "ohm/io/dlloader.h"
#include "ohm/vulkan/vulkan_impl.h"

namespace ohm {
namespace io {
class Dlloader;
}

struct InstanceData {};

namespace ovk {
class Instance {
 public:
  Instance();
  Instance(Instance&& mv);
  ~Instance();
  auto operator=(Instance&& mv) -> Instance&;
  auto initialize(io::Dlloader& loader, vk::AllocationCallbacks* callback)
      -> void;
  auto initialize(vk::Instance import, vk::PhysicalDevice import_device,
                  vk::AllocationCallbacks* allocation) -> void;
  auto setApplicationName(const char* name) -> void;
  auto setApplicationVersion(unsigned major, unsigned minor, unsigned revision)
      -> void;
  auto initialized() -> bool;
  auto addExtension(const char* extension_name) -> void;
  auto addValidationLayer(const char* layer_name) -> void;
  auto devices() -> const std::vector<vk::PhysicalDevice>&;
  auto instance() -> vk::Instance;
  auto dispatch() -> vk::DispatchLoaderDynamic&;
  auto device(unsigned id) -> vk::PhysicalDevice;

 private:
  std::vector<vk::PhysicalDevice> m_devices;
  std::array<unsigned, 3> version;
  std::string app_name;
  vk::DispatchLoaderDynamic m_dispatch;
  vk::Instance m_instance;
  std::vector<std::string> extensions;
  std::vector<std::string> validation;

  vk::ApplicationInfo makeAppInfo();

  vk::DebugUtilsMessengerCreateInfoEXT makeDebugInfo();

  std::vector<const char*> makeExtensionList();

  std::vector<const char*> makeValidationList();
};
}  // namespace ovk
}  // namespace ohm
