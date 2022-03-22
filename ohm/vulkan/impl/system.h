#pragma once
#include <string>

#include "api/system.h"

#include <vulkan/vulkan.hpp>
#include "device.h"
#include "instance.h"
#include "io/dlloader.h"
#include "memory.h"
#include "buffer.h"
#include "command_buffer.h"

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
  std::array<CommandBuffer, NUM_CACHE> commands;
  vk::AllocationCallbacks* allocate_cb;

  System() {
    this->system_name = "";
    this->allocate_cb = nullptr;
  }
  
  ~System() {
    for(auto& thing : this->buffer) {
      auto tmp = std::move(thing);
    }
    for(auto& thing : this->memory) {
      auto tmp = std::move(thing);
    }
    for(auto& thing : this->commands) {
      auto tmp = std::move(thing);
    }
  }
};

auto system() -> System&;

}  // namespace ovk
}  // namespace ohm