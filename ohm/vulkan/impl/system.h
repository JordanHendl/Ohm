#pragma once
#include <string>

#include "ohm/api/system.h"

#include <vulkan/vulkan.hpp>
#include "ohm/io/dlloader.h"
#include "ohm/vulkan/impl/buffer.h"
#include "ohm/vulkan/impl/command_buffer.h"
#include "ohm/vulkan/impl/descriptor.h"
#include "ohm/vulkan/impl/device.h"
#include "ohm/vulkan/impl/image.h"
#include "ohm/vulkan/impl/instance.h"
#include "ohm/vulkan/impl/memory.h"
#include "ohm/vulkan/impl/pipeline.h"

namespace ohm {
namespace ovk {
constexpr auto NUM_CACHE = 2048;

struct System {
  io::Dlloader loader;
  std::string system_name;
  Instance instance;
  std::vector<std::string> device_extensions;
  std::vector<std::string> validation_layers;
  std::vector<ovk::Device> devices;
  std::vector<ohm::Gpu> gpus;

  std::array<Memory, NUM_CACHE> memory;
  std::array<Buffer, NUM_CACHE> buffer;
  std::array<Image, NUM_CACHE> image;
  std::array<CommandBuffer, NUM_CACHE> commands;
  std::array<Pipeline, NUM_CACHE> pipeline;
  std::array<Descriptor, NUM_CACHE> descriptor;
  vk::AllocationCallbacks* allocate_cb;

  auto shutdown() -> void {
    for (auto& thing : this->image) {
      auto tmp = std::move(thing);
    }
    for (auto& thing : this->buffer) {
      auto tmp = std::move(thing);
    }
    for (auto& thing : this->memory) {
      auto tmp = std::move(thing);
    }
    for (auto& thing : this->commands) {
      auto tmp = std::move(thing);
    }
    for (auto& thing : this->pipeline) {
      auto tmp = std::move(thing);
    }
    for (auto& thing : this->devices) {
      ovk::clearPools(thing);
      auto tmp = std::move(thing);
    }
    { auto tmp = std::move(this->instance); }
    this->loader.reset();
  }

  System() {
    this->system_name = "";
    this->allocate_cb = nullptr;
  }

  ~System() { this->shutdown(); }
};

auto system() -> System&;

}  // namespace ovk
}  // namespace ohm