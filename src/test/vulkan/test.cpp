#include <gtest/gtest.h>

#include <iostream>
#include <memory>
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
}  // namespace memory
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
}

auto main(int argc, char* argv[]) -> int {
  ohm::System<ohm::API>::setDebugParameter("VK_LAYER_KHRONOS_validation");
  ohm::System<ohm::API>::setDebugParameter(
      "VK_LAYER_LUNARG_standard_validation");
  ohm::System<ohm::API>::initialize();
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
