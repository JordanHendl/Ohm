#define VULKAN_HPP_ASSERT_ON_RESULT
#define VULKAN_HPP_STORAGE_SHARED_EXPORT
#define VULKAN_HPP_STORAGE_SHARED
#define VULKAN_HPP_NO_DEFAULT_DISPATCHER
#define VULKAN_HPP_NO_EXCEPTIONS
#include "memory.h"
#include <algorithm>
#include <memory>
#include "device.h"
#include "error.h"
#include "instance.h"
#include "ohm/api/exception.h"
#include "system.h"

namespace ohm {
namespace ovk {
constexpr unsigned MIN_ALLOC_SIZE = 256;

auto memType(uint32_t filter, vk::MemoryPropertyFlags flag,
             vk::PhysicalDevice device) -> uint32_t {
  vk::PhysicalDeviceMemoryProperties mem_prop;

  mem_prop = device.getMemoryProperties(system().instance.dispatch());
  for (unsigned index = 0; index < mem_prop.memoryTypeCount; index++) {
    if (filter & (1 << index) &&
        (mem_prop.memoryTypes[index].propertyFlags & flag) == flag) {
      return index;
    }
  }
  return 0;
}

Memory::Memory() {
  this->size = 0;
  this->offset = 0;
  this->heap = 0;
  this->coherent = false;
  this->host_ptr = nullptr;
  this->device = nullptr;
  this->memory = nullptr;
}

Memory::Memory(Device& device, unsigned size, size_t heap, HeapType type) {
  this->device = &device;

  vk::MemoryAllocateInfo info;
  vk::MemoryPropertyFlags flag;

  size = std::max(size, MIN_ALLOC_SIZE);
  info.setAllocationSize(size);
  info.setMemoryTypeIndex(heap);

  this->memory =
      error(device.device().allocateMemory(info, nullptr, device.dispatch()));

  this->type = type;
  this->heap = heap;
  this->coherent = type & HeapType::HostVisible;
  this->size = size;
  this->offset = 0;
}

Memory::Memory(const Memory& parent, unsigned offset) {
  this->size = parent.size;
  this->coherent = parent.coherent;
  this->host_ptr = parent.host_ptr;
  this->memory = parent.memory;
  this->device = parent.device;
  this->heap = parent.heap;
  this->type = parent.type;

  this->offset = offset + parent.offset;
}

Memory::Memory(Memory&& mv) { *this = std::move(mv); }

Memory::~Memory() {
  if (this->initialized() && this->offset == 0) {
    this->device->device().free(this->memory, this->device->allocationCB(),
                                this->device->dispatch());
    this->size = 0;
    this->offset = 0;
    this->coherent = false;
    this->host_ptr = nullptr;
    this->memory = nullptr;
    this->device = nullptr;
    this->heap = 0;
  }
}

auto Memory::operator=(Memory&& mv) -> Memory& {
  this->size = mv.size;
  this->offset = mv.offset;
  this->coherent = mv.coherent;
  this->host_ptr = mv.host_ptr;
  this->memory = mv.memory;
  this->device = mv.device;
  this->heap = mv.heap;
  this->type = mv.type;

  mv.size = 0;
  mv.offset = 0;
  mv.coherent = false;
  mv.host_ptr = nullptr;
  mv.memory = nullptr;
  mv.device = nullptr;
  mv.heap = 0;
  return *this;
}

auto Memory::map(void** ptr) const -> void {
  OhmAssert(!this->initialized(),
            "Mapping an invalid memory object to host memory.");
  OhmAssert(!this->coherent,
            "Attempting to map memory that is not using a mappable heap.");

  vk::DeviceSize offset = this->offset;
  vk::DeviceSize amount = this->size - offset;
  vk::MemoryMapFlags flag;

  error(this->device->device().mapMemory(this->memory, offset, amount, flag,
                                         ptr, this->device->dispatch()));
  this->host_ptr = ptr;
}

auto Memory::unmap() const -> void {
  OhmAssert(!this->initialized() || !this->coherent || !this->host_ptr,
            "Error attempting to unmap memory.");
  this->device->device().unmapMemory(this->memory, this->device->dispatch());
}
}  // namespace ovk
}  // namespace ohm
