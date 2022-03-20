#pragma once

#include <array>
#include <climits>
#include <mutex>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>
#include "api/memory.h"
#include "vulkan/vulkan_impl.h"

namespace ohm {
namespace io {
class Dlloader;
}

namespace ovk {
class Instance;

struct Queue {
  vk::Queue queue;
  float priority = 1.0f;
  unsigned id = UINT_MAX;
  std::mutex lock;

  auto operator=(const Queue& cpy) -> Queue& {
    this->queue = cpy.queue;
    this->priority = cpy.priority;
    this->id = cpy.id;
    return *this;
  }
};

class Device {
 public:
  Device();
  Device(Device&& mv);
  auto operator=(const Device& cpy) -> Device&;
  auto initialize(io::Dlloader& loader, vk::AllocationCallbacks* callback,
                  vk::PhysicalDevice device) -> void;
  auto initialize(vk::Device import, io::Dlloader& loader,
                  vk::AllocationCallbacks* callback, vk::PhysicalDevice device)
      -> void;
  auto uuid() const -> unsigned long long;
  auto vendor() const -> unsigned long long;
  auto name() -> std::string_view;
  auto addExtension(const char* extension) -> void;
  auto addValidation(const char* validation) -> void;
  auto score() -> float;
  auto checkSupport(vk::SurfaceKHR surface) const -> void;
  auto device() const -> vk::Device;
  auto p_device() const -> vk::PhysicalDevice;
  auto graphics() -> Queue&;
  auto compute() -> Queue&;
  auto transfer() -> Queue&;
  auto sparse() -> Queue&;
  auto allocationCB() const -> vk::AllocationCallbacks*;
  auto dispatch() const -> vk::DispatchLoaderDynamic;
  auto memoryProperties() -> vk::PhysicalDeviceMemoryProperties&;
  auto heaps() const -> const std::vector<GpuMemoryHeap>&;

 private:
  vk::AllocationCallbacks* allocate_cb;
  vk::Device gpu;
  vk::PhysicalDevice physical_device;
  vk::SurfaceKHR surface;
  std::vector<vk::QueueFamilyProperties> queue_props;
  std::vector<GpuMemoryHeap> mem_heaps;
  vk::PhysicalDeviceProperties properties;
  vk::PhysicalDeviceFeatures features;
  vk::PhysicalDeviceMemoryProperties mem_prop;
  vk::DispatchLoaderDynamic m_dispatch;
  std::array<Queue, 4> queues;
  unsigned id;
  std::vector<std::string> extensions;
  std::vector<std::string> validation;
  float m_score;

  inline void findQueueFamilies();

  inline void makeDevice();

  inline std::vector<const char*> makeExtensions();

  inline std::vector<const char*> makeLayers();
};
}  // namespace ovk
}  // namespace ohm
