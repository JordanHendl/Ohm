#define VULKAN_HPP_ASSERT_ON_RESULT
#define VULKAN_HPP_STORAGE_SHARED_EXPORT
#define VULKAN_HPP_STORAGE_SHARED
#define VULKAN_HPP_NO_DEFAULT_DISPATCHER
#define VULKAN_HPP_NO_EXCEPTIONS

#include "ohm/vulkan/impl/descriptor.h"
#include <iostream>
#include "ohm/vulkan/impl/error.h"
#include "ohm/vulkan/impl/pipeline.h"

namespace ohm {
namespace ovk {
inline static auto convert(io::VariableType type) -> vk::DescriptorType;

auto convert(io::VariableType type) -> vk::DescriptorType {
  switch (type) {
    case io::VariableType::Sampler:
      return vk::DescriptorType::eCombinedImageSampler;
    case io::VariableType::Image:
      return vk::DescriptorType::eStorageImage;
    case io::VariableType::Uniform:
      return vk::DescriptorType::eUniformBuffer;
    case io::VariableType::Storage:
      return vk::DescriptorType::eStorageBuffer;
    default:
      return vk::DescriptorType::eUniformBuffer;
  }
}

DescriptorPool::DescriptorPool() {
  this->m_map = std::make_shared<UniformMap>();
  this->m_amount = 20;
  this->m_pool = nullptr;
  this->m_pipeline = nullptr;
}

DescriptorPool::DescriptorPool(DescriptorPool&& mv) { *this = std::move(mv); }

DescriptorPool::~DescriptorPool() {
  if (this->m_pool) {
    auto device = this->m_device->device();
    auto* alloc_cb = this->m_device->allocationCB();
    auto& dispatch = this->m_device->dispatch();
    auto flags = vk::DescriptorPoolResetFlags();
    error(device.resetDescriptorPool(this->m_pool, flags, dispatch));
    device.destroy(this->m_pool, alloc_cb, dispatch);
    this->m_pipeline = nullptr;
  }
}

auto DescriptorPool::operator=(DescriptorPool&& mv) -> DescriptorPool& {
  this->m_pool = mv.m_pool;
  this->m_amount = mv.m_amount;
  this->m_pipeline = mv.m_pipeline;
  this->m_layout = mv.m_layout;
  this->m_device = mv.m_device;

  mv.m_pool = nullptr;
  mv.m_layout = nullptr;
  mv.m_device = nullptr;
  mv.m_pipeline = nullptr;
  mv.m_amount = 20;

  this->m_map = std::move(mv.m_map);
  return *this;
}

auto DescriptorPool::initialize(const Pipeline& pipeline, size_t amount)
    -> void {
  const auto flag = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
  const auto& shader = pipeline.shader();

  this->m_amount = amount;
  this->m_pipeline = &pipeline;

  auto& map = *this->m_map;
  const auto& stages = shader.file().stages();
  if (!stages.empty()) {
    for (auto& stage : stages) {
      for (auto& variable : stage.variables) {
        map[variable.first] = variable.second;
      }
    }

    this->m_device = &shader.device();
    this->m_layout = shader.layout();

    vk::DescriptorPoolCreateInfo info;
    std::vector<vk::DescriptorPoolSize> sizes;
    vk::DescriptorPoolSize size;

    size.setDescriptorCount(this->m_amount);
    sizes.reserve(map.size());

    for (const auto& uniform : map) {
      size.setType(convert(uniform.second.type));
      sizes.push_back(size);
    }

    info.setPoolSizeCount(sizes.size());
    info.setPPoolSizes(sizes.data());
    info.setMaxSets(this->m_amount);
    info.setFlags(flag);

    if (info.poolSizeCount != 0) {
      auto device = this->m_device->device();
      auto& dispatch = this->m_device->dispatch();
      auto* alloc_cb = this->m_device->allocationCB();
      this->m_pool =
          error(device.createDescriptorPool(info, alloc_cb, dispatch));
    }
  }
}

Descriptor::Descriptor() {
  this->m_device = nullptr;
  this->m_pipeline = nullptr;
}

Descriptor::Descriptor(Descriptor&& mv) { *this = std::move(mv); }

Descriptor::Descriptor(DescriptorPool* pool) {
  this->m_device = nullptr;
  this->m_pipeline = nullptr;
  this->initialize(*pool);
}

Descriptor::~Descriptor() {}

auto Descriptor::operator=(Descriptor&& mv) -> Descriptor& {
  this->m_device = mv.m_device;
  this->m_parent_map = std::move(mv.m_parent_map);
  this->m_pipeline = mv.m_pipeline;
  this->m_set = mv.m_set;

  mv.m_set = nullptr;
  mv.m_device = nullptr;
  mv.m_pipeline = nullptr;
  return *this;
}

auto Descriptor::initialize(const DescriptorPool& pool) -> void {
  const vk::DescriptorSetLayout layouts[] = {pool.m_layout};
  vk::DescriptorSetAllocateInfo info;

  info.setDescriptorPool(pool.m_pool);
  info.setPSetLayouts(layouts);
  info.setDescriptorSetCount(1);

  if (pool.m_pool) {
    auto device = pool.m_device->device();
    auto& dispatch = pool.m_device->dispatch();
    auto result = error(device.allocateDescriptorSets(info, dispatch));
    this->m_device = pool.m_device;
    this->m_pipeline = pool.m_pipeline;
    this->m_parent_map = pool.m_map;
    this->m_set = result[0];
  }
}

auto Descriptor::bind(std::string_view name, const Buffer& buffer) -> void {
  if (this->m_parent_map) {
    const auto iter = this->m_parent_map->find(std::string(name));
    auto info = vk::DescriptorBufferInfo();
    auto write = vk::WriteDescriptorSet();

    if (iter != this->m_parent_map->end()) {
      info.setBuffer(buffer.buffer());
      info.setRange(VK_WHOLE_SIZE);
      info.setOffset(0);

      write.setDstSet(this->m_set);
      write.setDstBinding(iter->second.binding);
      write.setDescriptorType(convert(iter->second.type));
      write.setDstArrayElement(0);
      write.setDescriptorCount(1);
      write.setPBufferInfo(&info);

      auto device = this->m_device->device();
      auto& dispatch = this->m_device->dispatch();
      device.updateDescriptorSets(1, &write, 0, nullptr, dispatch);
    }
  }
}

auto Descriptor::bind(std::string_view name, const Image& image) -> void {
  if (this->m_parent_map) {
    const auto iter = this->m_parent_map->find(std::string(name));
    vk::DescriptorImageInfo info;
    vk::WriteDescriptorSet write;

    if (iter != this->m_parent_map->end()) {
      info.setImageLayout(image.layout());
      info.setSampler(image.sampler());
      info.setImageView(image.view());

      write.setDstSet(this->m_set);
      write.setDstBinding(iter->second.binding);
      write.setDescriptorType(convert(iter->second.type));
      write.setDstArrayElement(image.layer());
      write.setDescriptorCount(1);
      write.setPImageInfo(&info);

      auto device = this->m_device->device();
      auto& dispatch = this->m_device->dispatch();
      device.updateDescriptorSets(1, &write, 0, nullptr, dispatch);
    } else {
      OhmAssert(true, "Attempting to bind something that doesn't exist.");
    }
  }
}

auto Descriptor::bind(std::string_view name, const Image** images,
                      unsigned count) -> void {
  if (this->m_parent_map) {
    const auto iter = this->m_parent_map->find(std::string(name));
    unsigned amt;
    std::vector<vk::DescriptorImageInfo> infos;
    vk::WriteDescriptorSet write;

    if (iter != this->m_parent_map->end()) {
      amt = count < iter->second.size ? count : iter->second.size;

      infos.resize(count);
      for (unsigned index = 0; index < count; index++) {
        infos[index].setImageLayout(images[index]->layout());
        infos[index].setSampler(images[index]->sampler());
        infos[index].setImageView(images[index]->view());
      }

      write.setDstSet(this->m_set);
      write.setDstBinding(iter->second.binding);
      write.setDescriptorType(convert(iter->second.type));
      write.setDstArrayElement(0);
      write.setDescriptorCount(amt);
      write.setPImageInfo(infos.data());

      auto device = this->m_device->device();
      auto& dispatch = this->m_device->dispatch();
      device.updateDescriptorSets(1, &write, 0, nullptr, dispatch);
    } else {
      OhmAssert(true, "Attempting to bind something that doesn't exist.");
    }
  }
}

auto DescriptorPool::make() -> Descriptor { return Descriptor(this); }
}  // namespace ovk
}  // namespace ohm
