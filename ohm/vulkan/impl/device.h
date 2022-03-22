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

enum QueueTypes {
  GRAPHICS = 0,
  COMPUTE = 1,
  TRANSFER = 2,
  SPARSE = 3,
  NUM_QUEUES = 4
};

class Device {
 public:
  Device();
  Device(Device&& mv);
  ~Device();
  auto operator=(Device&& cpy) -> Device&;
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
  inline auto device() const -> vk::Device { return this->gpu; }
  inline auto p_device() const -> vk::PhysicalDevice {
    return this->physical_device;
  }
  inline auto graphics() -> Queue& { return this->queues[GRAPHICS]; }
  inline auto compute() -> Queue& { return this->queues[COMPUTE]; }
  inline auto transfer() -> Queue& { return this->queues[TRANSFER]; }
  inline auto sparse() -> Queue& { return this->queues[SPARSE]; }
  inline auto allocationCB() const -> vk::AllocationCallbacks* {
    return this->allocate_cb;
  }
  inline auto dispatch() const -> const vk::DispatchLoaderDynamic& {
    return this->m_dispatch;
  }
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
