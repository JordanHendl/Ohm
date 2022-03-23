#define VULKAN_HPP_ASSERT_ON_RESULT
#define VULKAN_HPP_STORAGE_SHARED_EXPORT
#define VULKAN_HPP_STORAGE_SHARED
#define VULKAN_HPP_NO_DEFAULT_DISPATCHER
#define VULKAN_HPP_NO_EXCEPTIONS

#include "vulkan_impl.h"
#include "ohm/api/exception.h"

#include "impl/memory.h"
#include "impl/system.h"

#include "ohm/api/memory.h"
#include "ohm/api/system.h"

#ifdef __linux__
#include <stdlib.h>
#include <unistd.h>
#elif _WIN32
//@JH TODO find out how to get system name from windows API.
#endif

namespace ohm {
inline namespace v1 {
auto Vulkan::System::initialize() Ohm_NOEXCEPT -> void {
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

auto Vulkan::System::name() Ohm_NOEXCEPT -> std::string {
#ifdef __linux__
  char hostname[HOST_NAME_MAX];
  gethostname(hostname, HOST_NAME_MAX);
  return std::string(hostname);
#elif _WIN32
  return "";
#endif
}

auto Vulkan::System::shutdown() Ohm_NOEXCEPT -> void {
  ovk::system().shutdown();
}

auto Vulkan::System::setParameter(std::string_view) Ohm_NOEXCEPT -> void {
  // nop... for now.
}

auto Vulkan::System::setDebugParameter(std::string_view str) Ohm_NOEXCEPT
    -> void {
  ovk::system().validation_layers.push_back(std::string(str));
  ovk::system().instance.addValidationLayer(str.cbegin());
}

auto Vulkan::System::devices() Ohm_NOEXCEPT -> std::vector<Gpu> {
  return ovk::system().gpus;
}

auto Vulkan::Memory::heaps(int gpu) Ohm_NOEXCEPT
    -> const std::vector<GpuMemoryHeap>& {
  auto& device = ovk::system().devices.at(gpu);
  return device.heaps();
}

//@JH TODO would like to simplify this as its a bit messy.
auto Vulkan::Memory::allocate(int gpu, HeapType requested, size_t heap_index,
                              size_t size) Ohm_NOEXCEPT -> int32_t {
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
          vk_type = vk_type | vk::MemoryPropertyFlagBits::eHostVisible |
                    vk::MemoryPropertyFlagBits::eHostCoherent;

        if (type.heapIndex == heap_index && (type.propertyFlags & vk_type)) {
          mem = std::move(ovk::Memory(device, size, type_index, requested));
          return index;
        }
      }
      OhmException(true, Error::APIError,
                   "Tried allocating to a heap that doesn't match requested "
                   "allocation type.");
    }
    index++;
  }

  OhmException(true, Error::APIError, "Too many memory allocations.");
  return 0;
}

auto Vulkan::Memory::type(int32_t handle) Ohm_NOEXCEPT -> HeapType {
  OhmException(handle < 0, Error::APIError, "Invalid handle passed to API.");
  auto& mem = ovk::system().memory[handle];
  OhmException(!mem.initialized(), Error::APIError,
               "Attempting to use memory object that is not initialized.");
  return mem.type;
}

auto Vulkan::Memory::destroy(int32_t handle) Ohm_NOEXCEPT -> void {
  OhmException(handle < 0, Error::APIError, "Invalid handle passed to API.");
  auto& mem = ovk::system().memory[handle];
  auto tmp = ovk::Memory();
  OhmException(!mem.initialized(), Error::APIError,
               "Attempting to use memory object that is not initialized.");
  tmp = std::move(mem);
}

auto Vulkan::Memory::size(int32_t handle) Ohm_NOEXCEPT -> size_t {
  OhmException(handle < 0, Error::APIError, "Invalid handle passed to API.");
  auto& mem = ovk::system().memory[handle];
  OhmException(!mem.initialized(), Error::APIError,
               "Attempting to use memory object that is not initialized.");
  return mem.size - mem.offset;
}

auto Vulkan::Memory::offset(int32_t handle, size_t offset) Ohm_NOEXCEPT
    -> size_t {
  OhmException(handle < 0, Error::APIError, "Invalid handle passed to API.");
  auto& parent = ovk::system().memory[handle];
  OhmException(!parent.initialized(), Error::APIError,
               "Attempting to use memory object that is not initialized.");
  auto index = 0;
  for (auto& mem : ovk::system().memory) {
    if (!mem.initialized()) {
      mem = std::move(ovk::Memory(parent, offset));
      return index;
    }
    index++;
  }

  OhmException(true, Error::APIError, "Too many memory allocations.");
  return 0;
}

auto Vulkan::Array::create(int gpu, size_t num_elmts,
                           size_t elm_size) Ohm_NOEXCEPT -> int32_t {
  auto& device = ovk::system().devices[gpu];
  auto index = 0;
  for (auto& buf : ovk::system().buffer) {
    if (!buf.initialized()) {
      buf = std::move(ovk::Buffer(device, num_elmts, elm_size));
      return index;
    }
    index++;
  }

  OhmException(true, Error::APIError,
               "Too many buffers. API has run out of allocation.");
  return -1;
}

auto Vulkan::Array::destroy(int32_t handle) Ohm_NOEXCEPT -> void {
  OhmException(handle < 0, Error::APIError,
               "Attempting to delete an invalid array handle.");
  auto& buf = ovk::system().buffer[handle];
  auto tmp = ovk::Buffer();

  OhmException(!buf.initialized(), Error::APIError,
               "Attempting to use array object that is not initialized.");
  tmp = std::move(buf);
}

auto Vulkan::Array::required(int32_t handle) Ohm_NOEXCEPT -> size_t {
  OhmException(handle < 0, Error::APIError,
               "Attempting to query an invalid array handle.");
  auto& buf = ovk::system().buffer[handle];

  OhmException(!buf.initialized(), Error::APIError,
               "Attempting to use array object that is not initialized.");
  return buf.size();
}

auto Vulkan::Array::bind(int32_t array_handle,
                         int32_t memory_handle) Ohm_NOEXCEPT -> void {
  OhmException(array_handle < 0, Error::APIError,
               "Attempting to bind to an invalid array handle.");
  OhmException(memory_handle < 0, Error::APIError,
               "Attempting to bind an invalid memory handle.");
  auto& buf = ovk::system().buffer[array_handle];
  auto& mem = ovk::system().memory[memory_handle];

  OhmException(!buf.initialized(), Error::APIError,
               "Attempting to use array object that is not initialized.");
  buf.bind(mem);
}

auto Vulkan::Image::create(int gpu, const ImageInfo& info) Ohm_NOEXCEPT
    -> int32_t {
  return -1;
}

auto Vulkan::Image::destroy(int32_t handle) Ohm_NOEXCEPT -> void {}

auto Vulkan::Image::layer(int32_t handle, size_t layer) Ohm_NOEXCEPT
    -> int32_t {
  return -1;
}

auto Vulkan::Image::required(int32_t handle) Ohm_NOEXCEPT -> size_t {
  return 1024;
}

auto Vulkan::Image::bind(int32_t image_handle, int32_t mem_handle) Ohm_NOEXCEPT
    -> void {}

auto Vulkan::Commands::create(int gpu, QueueType type) Ohm_NOEXCEPT -> int32_t {
  auto& device = ovk::system().devices[gpu];
  auto index = 0;
  for (auto& buf : ovk::system().commands) {
    if (!buf.initialized()) {
      buf = std::move(ovk::CommandBuffer(device, type));
      return index;
    }
    index++;
  }

  OhmException(true, Error::APIError,
               "Too many commands. API has run out of allocation.");
  return -1;
}

auto Vulkan::Commands::destroy(int32_t handle) Ohm_NOEXCEPT -> void {
  OhmException(handle < 0, Error::APIError,
               "Attempting to destroy an invalid commands handle.");
  auto& cmd = ovk::system().commands[handle];
  OhmException(!cmd.initialized(), Error::APIError,
               "Attempting to use object that is not initialized.");
  auto tmp = ovk::CommandBuffer();
  tmp = std::move(cmd);
}

auto Vulkan::Commands::begin(int32_t handle) Ohm_NOEXCEPT -> void {
  OhmException(handle < 0, Error::APIError,
               "Attempting to destroy an invalid commands handle.");
  auto& cmd = ovk::system().commands[handle];

  OhmException(!cmd.initialized(), Error::APIError,
               "Attempting to use object that is not initialized.");
  cmd.begin();
}

auto Vulkan::Commands::copyArray(int32_t handle, int32_t src, int32_t dst,
                                 size_t count) Ohm_NOEXCEPT -> void {
  OhmException(handle < 0, Error::APIError,
               "Attempting to use an invalid commands handle.");
  OhmException(src < 0, Error::APIError,
               "Attempting to use an invalid src array handle.");
  OhmException(dst < 0, Error::APIError,
               "Attempting to use an invalid dst array handle.");
  auto& cmd = ovk::system().commands[handle];

  auto& src_buf = ovk::system().buffer[src];
  auto& dst_buf = ovk::system().buffer[dst];

  OhmException(!cmd.initialized(), Error::APIError,
               "Attempting to use object that is not initialized.");
  cmd.copy(src_buf, dst_buf, count);
}

auto Vulkan::Commands::copyArray(int32_t handle, int32_t src, void* dst,
                                 size_t count) Ohm_NOEXCEPT -> void {
  OhmException(handle < 0, Error::APIError,
               "Attempting to use an invalid commands handle.");
  OhmException(src < 0, Error::APIError,
               "Attempting to use an invalid src array handle.");
  OhmException(dst == nullptr, Error::APIError,
               "Attempting to use an invalid dst ptr.");

  auto& cmd = ovk::system().commands[handle];
  auto& src_buf = ovk::system().buffer[src];

  OhmException(!cmd.initialized(), Error::APIError,
               "Attempting to use object that is not initialized.");
  cmd.copy(src_buf, static_cast<unsigned char*>(dst), count);
}

auto Vulkan::Commands::copyArray(int32_t handle, const void* src, int32_t dst,
                                 size_t count) Ohm_NOEXCEPT -> void {
  OhmException(handle < 0, Error::APIError,
               "Attempting to use an invalid commands handle.");
  OhmException(src == nullptr, Error::APIError,
               "Attempting to use an invalid src pointer.");
  OhmException(dst < 0, Error::APIError,
               "Attempting to use an invalid dst array handle.");

  auto& cmd = ovk::system().commands[handle];
  auto& dst_buf = ovk::system().buffer[dst];

  OhmException(!cmd.initialized(), Error::APIError,
               "Attempting to use object that is not initialized.");
  cmd.copy(static_cast<const unsigned char*>(src), dst_buf, count);
}

auto Vulkan::Commands::submit(int32_t handle) Ohm_NOEXCEPT -> void {
  OhmException(handle < 0, Error::APIError,
               "Attempting to use an invalid commands handle.");
  auto& cmd = ovk::system().commands[handle];

  OhmException(!cmd.initialized(), Error::APIError,
               "Attempting to use object that is not initialized.");
  cmd.submit();
}

auto Vulkan::Commands::synchronize(int32_t handle) Ohm_NOEXCEPT -> void {
  OhmException(handle < 0, Error::APIError,
               "Attempting to use an invalid commands handle.");
  auto& cmd = ovk::system().commands[handle];

  OhmException(!cmd.initialized(), Error::APIError,
               "Attempting to use object that is not initialized.");
  cmd.synchronize();
}
}  // namespace v1
}  // namespace ohm
