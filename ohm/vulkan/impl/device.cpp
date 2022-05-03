#define VULKAN_HPP_ASSERT_ON_RESULT
#define VULKAN_HPP_STORAGE_SHARED_EXPORT
#define VULKAN_HPP_STORAGE_SHARED
#define VULKAN_HPP_NO_DEFAULT_DISPATCHER
#define VULKAN_HPP_NO_EXCEPTIONS

#include "ohm/vulkan/impl/device.h"
#include <memory>
#include "ohm/api/exception.h"
#include "ohm/io/dlloader.h"
#include "ohm/vulkan/impl/error.h"
#include "ohm/vulkan/impl/instance.h"
#include "ohm/vulkan/impl/system.h"

namespace ohm {
namespace ovk {

Device::Device() {
  this->extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  this->extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
  this->allocate_cb = nullptr;
  this->m_score = 0.0f;
}

Device::Device(Device&& mv) { *this = std::move(mv); }

Device::~Device() {
  if (this->gpu) {
    this->gpu.destroy(this->allocate_cb, ovk::system().instance.dispatch());
  }
}

auto Device::operator=(Device&& mv) -> Device& {
  this->allocate_cb = mv.allocate_cb;
  this->gpu = mv.gpu;
  this->physical_device = mv.physical_device;
  this->surface = mv.surface;
  this->queue_props = mv.queue_props;
  this->properties = mv.properties;
  this->mem_prop = mv.mem_prop;
  this->mem_heaps = mv.mem_heaps;
  this->features = mv.features;
  this->m_dispatch = mv.m_dispatch;
  this->queues = mv.queues;
  this->id = mv.id;
  this->extensions = mv.extensions;
  this->validation = mv.validation;
  this->m_score = mv.m_score;

  mv.allocate_cb = nullptr;
  mv.gpu = nullptr;
  mv.physical_device = nullptr;
  mv.surface = nullptr;
  mv.mem_prop = vk::PhysicalDeviceMemoryProperties();
  mv.mem_heaps = {};
  mv.properties = vk::PhysicalDeviceProperties();
  mv.features = vk::PhysicalDeviceFeatures();
  mv.id = 0;
  mv.m_score = 0.f;
  mv.queue_props.clear();
  mv.extensions.clear();
  mv.validation.clear();
  for (auto& q : mv.queues) q = Queue();

  return *this;
}

auto Device::findQueueFamilies() -> void {
  this->queue_props = this->physical_device.getQueueFamilyProperties(
      system().instance.dispatch());

  for (unsigned id = 0; id < this->queue_props.size(); id++) {
    auto& family = this->queue_props[id];

    auto queue_flag = static_cast<VkQueueFlags>(family.queueFlags);

    if (queue_flag & VK_QUEUE_GRAPHICS_BIT && id != this->queues[COMPUTE].id &&
        this->queues[GRAPHICS].id == UINT_MAX) {
      this->queues[GRAPHICS].id = id;
    } else if (queue_flag & VK_QUEUE_COMPUTE_BIT &&
               id != this->queues[GRAPHICS].id &&
               this->queues[COMPUTE].id == UINT_MAX) {
      this->queues[COMPUTE].id = id;
    } else if (queue_flag & VK_QUEUE_TRANSFER_BIT &&
               id != this->queues[SPARSE].id &&
               this->queues[TRANSFER].id == UINT_MAX) {
      this->queues[TRANSFER].id = id;
    } else if (queue_flag & VK_QUEUE_SPARSE_BINDING_BIT &&
               this->queues[SPARSE].id != UINT_MAX) {
      this->queues[SPARSE].id = id;
    }
  }
}

auto Device::makeDevice() -> void {
  using StringVec = std::vector<const char*>;
  using QueueInfos = std::vector<vk::DeviceQueueCreateInfo>;

  vk::DeviceCreateInfo info;
  StringVec extensions;
  StringVec validation;
  QueueInfos queue_infos;

  extensions = this->makeExtensions();
  validation = this->makeLayers();

  for (const auto& queue : this->queues) {
    if (queue.id != UINT_MAX) {
      vk::DeviceQueueCreateInfo info;

      info.setQueueFamilyIndex(queue.id);
      info.setQueueCount(1);
      info.setPQueuePriorities(&queue.priority);

      queue_infos.push_back(info);
    }
  }

  vk::PhysicalDeviceProperties2 props;

  this->features.setShaderInt64(true);
  this->features.setFragmentStoresAndAtomics(true);
  this->features.setVertexPipelineStoresAndAtomics(true);
  info.setQueueCreateInfos(queue_infos);
  info.setEnabledExtensionCount(extensions.size());
  info.setPpEnabledExtensionNames(extensions.data());
  info.setEnabledLayerCount(validation.size());
  info.setPpEnabledLayerNames(validation.data());
  info.setPEnabledFeatures(&this->features);
  //      info.setPNext                  ( &this->pnext_chain ) ;
  error(this->physical_device.createDevice(&info, this->allocate_cb, &this->gpu,
                                           system().instance.dispatch()));
}

auto Device::makeExtensions() -> std::vector<const char*> {
  std::vector<const char*> list;

  auto available_extentions =
      error(this->physical_device.enumerateDeviceExtensionProperties(
          nullptr, system().instance.dispatch()));

  auto copy = this->extensions;

  this->extensions.clear();
  for (const auto& ext : available_extentions) {
    for (const auto& requested : copy) {
      if (std::string(&ext.extensionName[0]) == requested) {
        this->extensions.push_back(requested);
        list.push_back(this->extensions.back().data());
      }
    }
  }

  return list;
}

auto Device::makeLayers() -> std::vector<const char*> {
  std::vector<const char*> list;
  auto available_layers =
      error(this->physical_device.enumerateDeviceLayerProperties(
          system().instance.dispatch()));
  auto copy = this->validation;

  this->validation.clear();
  for (const auto& ext : available_layers) {
    for (const auto& requested : copy) {
      if (std::string(&ext.layerName[0]) == requested) {
        this->validation.push_back(requested);
        list.push_back(this->validation.back().data());
      }
    }
  }

  return list;
}

auto Device::initialize(io::Dlloader& loader, vk::AllocationCallbacks* callback,
                        vk::PhysicalDevice device) -> void {
  this->physical_device = device;
  this->allocate_cb = callback;

  this->properties = device.getProperties(system().instance.dispatch());
  if(this->properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
    this->m_score += 50;
  }
  
  this->findQueueFamilies();
  this->makeDevice();

  this->mem_prop = device.getMemoryProperties(system().instance.dispatch());

  this->mem_heaps.resize(mem_prop.memoryHeapCount);
  for (auto index = 0u; index < mem_prop.memoryTypeCount; index++) {
    auto& vk_type = this->mem_prop.memoryTypes[index];
    auto& vk_heap = this->mem_prop.memoryHeaps[vk_type.heapIndex];
    auto& heap = this->mem_heaps[vk_type.heapIndex];

    heap.size = vk_heap.size;
    auto host_visible =
        vk_type.propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible ||
        vk_type.propertyFlags & vk::MemoryPropertyFlagBits::eHostCoherent;

    auto device_capable =
        vk_type.propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal;

    if (host_visible) heap.type = HeapType::HostVisible | heap.type;
    if (device_capable) heap.type = HeapType::GpuOnly | heap.type;
  }

  this->m_dispatch.init(system().instance.instance(),
                        reinterpret_cast<PFN_vkGetInstanceProcAddr>(
                            loader.symbol("vkGetInstanceProcAddr")),
                        this->gpu);

  /* Check and see if we found Queues. If not, set to the 'last available queue'
   * as to not break if people need to use say, a compute queue even though
   * teeeeeechnically their device doesn't support it.
   */
  if (this->queues[GRAPHICS].id != UINT_MAX) {
    this->queues[GRAPHICS].queue =
        this->gpu.getQueue(this->queues[GRAPHICS].id, 0, this->m_dispatch);
  }

  // Set graphics to the defacto default because this queue is guaranteed to be
  // able to to everything + graphics.
  auto last_queue = this->queues[GRAPHICS].queue;

  if (this->queues[COMPUTE].id != UINT_MAX) {
    this->queues[COMPUTE].queue =
        this->gpu.getQueue(this->queues[COMPUTE].id, 0, this->m_dispatch);
    last_queue = this->queues[COMPUTE].queue;
  } else {
    this->queues[COMPUTE].queue = last_queue;
  }

  if (this->queues[TRANSFER].id != UINT_MAX) {
    this->queues[TRANSFER].queue =
        this->gpu.getQueue(this->queues[TRANSFER].id, 0, this->m_dispatch);
    last_queue = this->queues[TRANSFER].queue;
  } else {
    this->queues[COMPUTE].queue = last_queue;
  }

  if (this->queues[SPARSE].id != UINT_MAX) {
    this->queues[SPARSE].queue =
        this->gpu.getQueue(this->queues[SPARSE].id, 0, this->m_dispatch);
    last_queue = this->queues[SPARSE].queue;
  } else {
    this->queues[SPARSE].queue = last_queue;
  }
}

auto Device::initialize(vk::Device import, io::Dlloader& loader,
                        vk::AllocationCallbacks* callback,
                        vk::PhysicalDevice device) -> void {
  this->physical_device = device;
  this->allocate_cb = callback;

  this->findQueueFamilies();
  this->gpu = import;
  this->m_dispatch.init(system().instance.instance(),
                        reinterpret_cast<PFN_vkGetInstanceProcAddr>(
                            loader.symbol("vkGetInstanceProcAddr")),
                        this->gpu);
  this->queues[GRAPHICS].queue =
      this->gpu.getQueue(this->queues[GRAPHICS].id, 0, this->m_dispatch);
  this->queues[COMPUTE].queue =
      this->gpu.getQueue(this->queues[COMPUTE].id, 0, this->m_dispatch);
  this->queues[TRANSFER].queue =
      this->gpu.getQueue(this->queues[TRANSFER].id, 0, this->m_dispatch);

  if (this->queues[SPARSE].id != UINT_MAX)
    this->queues[SPARSE].queue =
        this->gpu.getQueue(this->queues[SPARSE].id, 0, this->m_dispatch);
}

auto Device::uuid() const -> unsigned long long {
  return this->properties.deviceID;
}

auto Device::vendor() const -> unsigned long long {
  return this->properties.vendorID;
}

auto Device::addExtension(const char* extension) -> void {
  this->extensions.push_back(extension);
}

auto Device::addValidation(const char* validation) -> void {
  this->validation.push_back(validation);
}

auto Device::score() -> float { return this->m_score; }

auto Device::checkSupport(vk::SurfaceKHR surface) const -> void {
  error(this->p_device().getSurfaceSupportKHR(this->queues[GRAPHICS].id,
                                              surface, this->dispatch()));
}

auto Device::memoryProperties() -> vk::PhysicalDeviceMemoryProperties& {
  return this->mem_prop;
}

auto Device::heaps() const -> const std::vector<GpuMemoryHeap>& {
  return this->mem_heaps;
}
}  // namespace ovk
}  // namespace ohm
