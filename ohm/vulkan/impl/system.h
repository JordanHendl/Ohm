#pragma once
#include <string>
#include <unordered_map>

#include <vulkan/vulkan.hpp>
#include "ohm/api/system.h"
#include "ohm/io/dlloader.h"
#include "ohm/vulkan/impl/buffer.h"
#include "ohm/vulkan/impl/command_buffer.h"
#include "ohm/vulkan/impl/descriptor.h"
#include "ohm/vulkan/impl/device.h"
#include "ohm/vulkan/impl/image.h"
#include "ohm/vulkan/impl/instance.h"
#include "ohm/vulkan/impl/memory.h"
#include "ohm/vulkan/impl/pipeline.h"
#include "ohm/vulkan/impl/swapchain.h"
#include "ohm/vulkan/impl/event.h"
#include "ohm/vulkan/impl/window.h"
namespace ohm {
namespace ovk {
constexpr auto CACHE_SIZE = 2048;
constexpr auto NUM_WINDOWS = 4;
struct System {
  io::Dlloader loader;
  std::string system_name;
  Instance instance;
  std::vector<std::string> device_extensions;
  std::vector<std::string> validation_layers;
  std::vector<ovk::Device> devices;
  std::vector<ohm::Gpu> gpus;

  std::array<Memory, CACHE_SIZE> memory;
  std::array<Buffer, CACHE_SIZE> buffer;
  std::array<Image, CACHE_SIZE> image;
  std::array<CommandBuffer, CACHE_SIZE> commands;
  std::array<Pipeline, CACHE_SIZE> pipeline;
  std::array<Descriptor, CACHE_SIZE> descriptor;
  std::array<Window, NUM_WINDOWS> window;
  std::array<Swapchain, NUM_WINDOWS> swapchain;
  vk::AllocationCallbacks* allocate_cb;

  std::unordered_map<int32_t, std::shared_ptr<Event>> event;
  
  auto shutdown() -> void {
    for (auto& thing : this->swapchain) {
      auto tmp = std::move(thing);
    }
    for (auto& thing : this->window) {
      auto tmp = std::move(thing);
    }
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