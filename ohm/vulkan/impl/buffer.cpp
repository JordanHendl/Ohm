#define VULKAN_HPP_ASSERT_ON_RESULT
#define VULKAN_HPP_STORAGE_SHARED_EXPORT
#define VULKAN_HPP_STORAGE_SHARED
#define VULKAN_HPP_NO_DEFAULT_DISPATCHER
#define VULKAN_HPP_NO_EXCEPTIONS

#include "buffer.h"
#include "device.h"
#include "error.h"
#include "instance.h"
#include "memory.h"
#include "system.h"

#include "api/exception.h"

#include <memory>
#include <vulkan/vulkan.hpp>

namespace ohm {
namespace ovk {
Buffer::Buffer() {
  this->m_device = nullptr;
  this->m_memory = nullptr;
  this->m_buffer = nullptr;
  this->m_count = 0;
  this->m_element_size = 0;
  this->m_flags = vk::BufferUsageFlags();
  this->m_requirements = vk::MemoryRequirements();
}

Buffer::Buffer(Device& device, size_t count, size_t size) {
  this->m_memory = nullptr;
  this->m_device = &device;
  this->m_count = count;

  //@JH TODO Make this configurable. Will work for now, but is not the most
  //efficient.
  this->m_flags |= vk::BufferUsageFlagBits::eStorageBuffer;
  this->m_flags |= vk::BufferUsageFlagBits::eUniformBuffer;
  this->m_flags |= vk::BufferUsageFlagBits::eIndexBuffer;
  this->m_flags |= vk::BufferUsageFlagBits::eVertexBuffer;
  this->m_flags |= vk::BufferUsageFlagBits::eTransferSrc;
  this->m_flags |= vk::BufferUsageFlagBits::eTransferDst;

  this->m_element_size = size;
  this->createBuffer(size * count);
  this->m_requirements = device.device().getBufferMemoryRequirements(
      this->m_buffer, device.dispatch());
}

Buffer::Buffer(Buffer&& mv) { *this = std::move(mv); }

Buffer::~Buffer() {
  if (this->initialized()) {
    const auto& device = this->m_device->device();
    device.destroy(this->m_buffer, this->m_device->allocationCB(),
                   this->m_device->dispatch());
  }
}

auto Buffer::operator=(Buffer&& mv) -> Buffer& {
  this->m_device = mv.m_device;
  this->m_memory = mv.m_memory;
  this->m_buffer = mv.m_buffer;
  this->m_count = mv.m_count;
  this->m_element_size = mv.m_element_size;
  this->m_flags = mv.m_flags;
  this->m_requirements = mv.m_requirements;

  mv.m_device = nullptr;
  mv.m_memory = nullptr;
  mv.m_buffer = nullptr;
  mv.m_count = 0;
  mv.m_element_size = 0;
  mv.m_flags = vk::BufferUsageFlags();
  mv.m_requirements = vk::MemoryRequirements();

  return *this;
}

void Buffer::createBuffer(unsigned size) {
  vk::BufferCreateInfo info;

  info.setSize(size);
  info.setUsage(this->m_flags);
  info.setSharingMode(vk::SharingMode::eExclusive);

  this->m_buffer = error(this->m_device->device().createBuffer(
      info, this->m_device->allocationCB(), this->m_device->dispatch()));
}

void Buffer::bind(Memory& memory) {
  OhmException(memory.size < this->m_requirements.size, Error::APIError,
               "Attempting to bind memory that is of inadequate size.");
  error(this->m_device->device().bindBufferMemory(this->m_buffer, memory.memory,
                                                  memory.offset,
                                                  this->m_device->dispatch()));
  this->m_memory = &memory;
}
}  // namespace ovk
}  // namespace ohm
