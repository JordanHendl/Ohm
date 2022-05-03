#pragma once
#include <string>
#include <unordered_map>

#include <vulkan/vulkan.hpp>
#include "ohm/api/system.h"
#include "ohm/io/dlloader.h"
#include "ohm/vulkan/impl/render_pass.h"
#include "ohm/vulkan/impl/buffer.h"
#include "ohm/vulkan/impl/command_buffer.h"
#include "ohm/vulkan/impl/descriptor.h"
#include "ohm/vulkan/impl/device.h"
#include "ohm/vulkan/impl/event.h"
#include "ohm/vulkan/impl/image.h"
#include "ohm/vulkan/impl/instance.h"
#include "ohm/vulkan/impl/memory.h"
#include "ohm/vulkan/impl/pipeline.h"
#include "ohm/vulkan/impl/swapchain.h"
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

  std::vector<Memory> memory;
  std::vector<Buffer> buffer;
  std::vector<Image> image;
  std::vector<CommandBuffer> commands;
  std::vector<RenderPass> render_pass;
  std::vector<Pipeline> pipeline;
  std::vector<Descriptor> descriptor;
  std::vector<Window> window;
  std::vector<Swapchain> swapchain;
  vk::AllocationCallbacks* allocate_cb;
  std::unordered_map<int32_t, std::shared_ptr<Event>> event;

  auto shutdown() -> void {
    this->swapchain.clear();
    this->window.clear();
    this->image.clear();
    this->render_pass.clear();
    this->buffer.clear();
    this->memory.clear();
    this->descriptor.clear();
    this->commands.clear();
    this->pipeline.clear();
    
    for (auto& thing : this->devices) {
      ovk::clearPools(thing);
      auto tmp = std::move(thing);
    }

    this->devices.clear();
    { auto tmp = std::move(this->instance); }
    this->loader.reset();
  }

  System() {
    this->memory.resize(CACHE_SIZE);
    this->buffer.resize(CACHE_SIZE);
    this->image.resize(CACHE_SIZE);
    this->commands.resize(CACHE_SIZE);
    this->render_pass.resize(CACHE_SIZE);
    this->pipeline.resize(CACHE_SIZE);
    this->descriptor.resize(CACHE_SIZE);
    this->window.resize(NUM_WINDOWS);
    this->swapchain.resize(NUM_WINDOWS);
    this->system_name.clear();
    this->device_extensions.clear();
    this->devices.clear();
    this->allocate_cb = nullptr;
  }

  ~System() { this->shutdown(); }
};
auto system() -> System&;
}  // namespace ovk
}  // namespace ohm