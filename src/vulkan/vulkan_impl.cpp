#define VULKAN_HPP_ASSERT_ON_RESULT
#define VULKAN_HPP_STORAGE_SHARED_EXPORT
#define VULKAN_HPP_STORAGE_SHARED
#define VULKAN_HPP_NO_DEFAULT_DISPATCHER
#define VULKAN_HPP_NO_EXCEPTIONS

#include "vulkan_impl.h"
#include "api/exception.h"

#include "impl/memory.h"
#include "impl/system.h"

#include "api/memory.h"
#include "api/system.h"

#ifdef __linux__
#include <stdlib.h>
#include <unistd.h>
#elif _WIN32
//@JH TODO find out how to get system name from windows API.
#endif

namespace ohm {
auto Vulkan::System::initialize() -> void {
  auto& loader = ovk::system().loader;
  if (!loader.initialized()) {
#ifdef _WIN32
    loader.load("vulkan-1.dll");
#elif __linux__
    loader.load("libvulkan.so.1");
#endif

    ovk::system().instance.initialize(loader, ovk::system().allocate_cb);
    ovk::system().devices.resize(ovk::system().instance.devices().size());
    ovk::system().gpus.resize(ovk::system().devices.size());

    unsigned index = 0;
    for (auto& device : ovk::system().devices) {
      auto& gpu = ovk::system().gpus[index];
      for (auto& extension : ovk::system().device_extensions) {
        device.addExtension(extension.c_str());
      }

      for (auto& validation : ovk::system().validation_layers) {
        device.addValidation(validation.c_str());
      }

      device.initialize(loader, ovk::system().allocate_cb,
                        ovk::system().instance.device(index++));
      gpu.name = device.name();
    }
  }
}

auto Vulkan::System::name() -> std::string {
#ifdef __linux__
  char hostname[HOST_NAME_MAX];
  gethostname(hostname, HOST_NAME_MAX);
  return std::string(hostname);
#elif _WIN32
  return "";
#endif
}

auto Vulkan::System::setParameter(std::string_view) -> void {
  // nop... for now.
}

auto Vulkan::System::setDebugParameter(std::string_view str) -> void {
  ovk::system().validation_layers.push_back(std::string(str));
  ovk::system().instance.addValidationLayer(str.cbegin());
}

auto Vulkan::System::devices() -> std::vector<Gpu> {
  return ovk::system().gpus;
}

auto Vulkan::Memory::heaps(int gpu) -> const std::vector<GpuMemoryHeap>& {
  auto& device = ovk::system().devices.at(gpu);
  return device.heaps();
}

auto Vulkan::Memory::allocate(int gpu, HeapType requested, size_t heap_index,
                              size_t size) -> int32_t {
  auto& device = ovk::system().devices[gpu];
  auto mem_type_count = device.memoryProperties().memoryTypeCount;
  auto index = 0;
  for (auto& mem : ovk::system().memory) {
    if (!mem.initialized()) {
      for (auto type_index = 0u; type_index < mem_type_count; type_index++) {
        auto& type = device.memoryProperties().memoryTypes[type_index];
        auto vk_type = vk::MemoryPropertyFlags();
        if (requested & HeapType::GpuOnly)
          vk_type = vk_type | vk::MemoryPropertyFlagBits::eDeviceLocal;
        if (requested & HeapType::HostVisible)
          vk_type = vk_type | vk::MemoryPropertyFlagBits::eHostVisible;

        if (type.heapIndex == heap_index && type.propertyFlags & vk_type) {
          mem = std::move(ovk::Memory(device, size, heap_index, requested));
          return index;
        }
      }
      OhmException(true, Error::LogicError,
                   "Tried allocating to a heap that doesn't match requested "
                   "allocation type.");
    }
    index++;
  }

  OhmException(true, Error::LogicError, "Too many memory allocations.");
  return 0;
}

auto Vulkan::Memory::type(int32_t handle) -> HeapType {
  OhmException(handle < 0, Error::LogicError, "Invalid handle passed to API.");
  auto& mem = ovk::system().memory[handle];
  return mem.type;
}

auto Vulkan::Memory::destroy(int32_t handle) -> void {
  OhmException(handle < 0, Error::LogicError, "Invalid handle passed to API.");
  auto& mem = ovk::system().memory[handle];
  auto tmp = ovk::Memory();
  tmp = std::move(mem);
}

auto Vulkan::Memory::size(int32_t handle) -> size_t {
  OhmException(handle < 0, Error::LogicError, "Invalid handle passed to API.");
  auto& mem = ovk::system().memory[handle];
  return mem.size - mem.offset;
}

auto Vulkan::Memory::offset(int32_t handle, size_t offset) -> size_t {
  OhmException(handle < 0, Error::LogicError, "Invalid handle passed to API.");
  auto& parent = ovk::system().memory[handle];
  auto index = 0;
  for (auto& mem : ovk::system().memory) {
    if (!mem.initialized()) {
      mem = std::move(ovk::Memory(parent, offset));
      return index;
    }
    index++;
  }

  OhmException(true, Error::LogicError, "Too many memory allocations.");
  return 0;
}
}  // namespace ohm
