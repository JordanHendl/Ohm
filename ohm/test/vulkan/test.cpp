#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <array>
#include "api/bits/pool_allocator.h"
#include "api/ohm.h"
#include "vulkan/vulkan_impl.h"

namespace ohm {
using API = ohm::Vulkan;

namespace sys {

auto test_name() -> bool {
  auto name = System<API>::name();
  return !name.empty();
}

auto test_devices() -> bool {
  auto devices = System<API>::devices();
  return !devices.empty();
}

auto test_set_param() -> bool {
  System<API>::setParameter("lmao");
  return true;
}
}  // namespace sys

namespace memory {
auto test_allocation() -> bool {
  constexpr auto mem_size = 1024;
  auto memory = Memory<API>(0, mem_size);
  return memory.gpu() == 0;
}

auto test_size() -> bool {
  constexpr auto mem_size = 1024;
  auto memory = Memory<API>(0, mem_size);
  return memory.size() >= mem_size;
}

auto test_type() -> bool {
  constexpr auto mem_size = 1024;
  auto memory = Memory<API>(0, HeapType::HostVisible, mem_size);
  return memory.type() == HeapType::HostVisible;
}

auto test_offset() -> bool {
  constexpr auto mem_size = 2048;
  auto memory_1 = Memory<API>(0, HeapType::GpuOnly, mem_size);
  auto memory_2 = Memory<API>(memory_1, 1024);

  auto same_gpu = memory_2.gpu() == memory_1.gpu();
  auto correct_size = memory_2.size() == 1024;
  return same_gpu && correct_size;
}

auto test_pool() -> bool {
  constexpr auto mem_size = 1024;
  auto memory =
      Memory<API, PoolAllocator<API>>(0, HeapType::HostVisible, mem_size);
  return memory.type() & HeapType::HostVisible;
}

auto test_move() -> bool {
  constexpr auto mem_size = 1024;
  auto memory_1 = Memory<API, PoolAllocator<API>>(0, HeapType::HostVisible, mem_size);
  auto memory_2 = Memory<API, PoolAllocator<API>>(std::move(memory_1));
  
  return memory_1.handle() < 0 && memory_2.handle() >= 0 && memory_2.type() & HeapType::HostVisible;
}
}  // namespace memory

namespace array {
auto test_allocation() -> bool {
  auto array = Array<API, float>(0, 64);
  return array.size() > 0 && array.handle() >= 0;
}

auto test_allocation_from_memory() -> bool {
  auto memory = Memory<API>(0, 1024);
  auto array = Array<API, float>(std::move(memory), 64);
  return array.size() > 0 && array.handle() >= 0;
}

auto test_mapped_allocation() -> bool {
  auto array = Array<API, float>(0, 1024, HeapType::HostVisible);
  return array.size() > 0 && array.handle() >= 0;
}
}

namespace commands {
auto test_creation() -> bool {
  auto commands = Commands<API>(0);
  return commands.handle() >= 0;
}

auto test_synchronize() -> bool {
  auto commands = Commands<API>(0);
  commands.synchronize();
  return true;
}

auto test_host_to_array_copy() -> bool {
  auto commands = Commands<API>(0);
  auto array = Array<API, int>(0, 1024, HeapType::HostVisible);
  std::array<int, 1024> host_array;
  
  for(auto& num : host_array) {
    num = 1337;
  }
  
  commands.begin();
  commands.copy(host_array.data(), array);
  commands.submit();
  
  for(auto& num : host_array) {
    num = 0;
  }
  
  commands.begin();
  commands.copy(array, host_array.data());
  commands.submit();
  
  for(auto& num : host_array) {
    if(num != 1337) return false;
  }
  return true;
}

auto test_gpu_array_copy() -> bool {
  auto commands = Commands<API>(0);
  constexpr auto cache_size = 1024;
  auto array_host = Array<API, int>(0, cache_size, HeapType::HostVisible);
  auto array_1 = Array<API, int>(0, cache_size, HeapType::GpuOnly);
  auto array_2 = Array<API, int>(0, cache_size, HeapType::GpuOnly);
  std::array<int, cache_size> host_array;
  
  for(auto& num : host_array) {
    num = 1337;
  }
  
  commands.copy(host_array.data(), array_host);
  commands.copy(array_host, array_1);
  commands.copy(array_host, array_2);
  commands.submit();
  commands.synchronize();
  
  for(auto& num : host_array) {
    num = 0;
  }
  
  commands.begin();
  commands.copy(host_array.data(), array_host);
  commands.copy(array_host, array_1);
  commands.copy(array_2, array_host);
  commands.submit();
  commands.synchronize();
  
  commands.copy(array_host, host_array.data());
  
  for(auto& num : host_array) {
    if(num != 1337)
      return false;
  }
  
  commands.begin();
  commands.copy(array_1, array_host);
  commands.submit();
  commands.synchronize();
  
  commands.copy(array_host, host_array.data());
  
  for(auto& num : host_array) {
    if(num != 0)
      return false;
  }
  
  return true;
}
}
}  // namespace ohm

TEST(Vulkan, System) {
  EXPECT_TRUE(ohm::sys::test_name());
  EXPECT_TRUE(ohm::sys::test_devices());
  EXPECT_TRUE(ohm::sys::test_set_param());
}

TEST(Vulkan, Memory) {
  EXPECT_TRUE(ohm::memory::test_allocation());
  EXPECT_TRUE(ohm::memory::test_size());
  EXPECT_TRUE(ohm::memory::test_type());
  EXPECT_TRUE(ohm::memory::test_offset());
  EXPECT_TRUE(ohm::memory::test_pool());
  EXPECT_TRUE(ohm::memory::test_move());
}

TEST(Vulkan, Array) {
  EXPECT_TRUE(ohm::array::test_allocation());
  EXPECT_TRUE(ohm::array::test_allocation_from_memory());
  EXPECT_TRUE(ohm::array::test_mapped_allocation());
}

TEST(Vulkan, Commands) {
  EXPECT_TRUE(ohm::commands::test_creation());
  EXPECT_TRUE(ohm::commands::test_host_to_array_copy());
  EXPECT_TRUE(ohm::commands::test_gpu_array_copy());
}

auto main(int argc, char* argv[]) -> int {
#ifdef Ohm_Debug
  ohm::System<ohm::API>::setDebugParameter("VK_LAYER_KHRONOS_validation");
  ohm::System<ohm::API>::setDebugParameter(
      "VK_LAYER_LUNARG_standard_validation");
#endif
  ohm::System<ohm::API>::initialize();
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
