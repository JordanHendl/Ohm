#include <gtest/gtest.h>
#include <array>
#include <iostream>
#include <memory>
#include "ohm/api/ohm.h"
#include "ohm/vulkan/vulkan_impl.h"

namespace ohm {
using API = ohm::Vulkan;
const char* test_compute_shader = {
    "#version 450 core\n"
    "#extension GL_ARB_separate_shader_objects : enable\n"
    "#extension GL_GOOGLE_include_directive    : enable\n"
    "#define BLOCK_SIZE_X 32\n"
    "#define BLOCK_SIZE_Y 32\n"
    "#define BLOCK_SIZE_Z 1 \n"
    "layout( local_size_x = BLOCK_SIZE_X, local_size_y = BLOCK_SIZE_Y, "
    "local_size_z = BLOCK_SIZE_Z ) in ; \n"
    "layout( binding = 0, rgba32f ) coherent restrict readonly  uniform "
    "image2D input_tex ;\n"
    "layout( binding = 1, rgba32f ) coherent restrict writeonly uniform "
    "image2D output_tex;\n"
    "layout( binding = 3 ) buffer Configuration\n"
    "{\n"
    "  float threshold;\n"
    "} config;\n"
    "void main()\n"
    "{\n"
    "  const ivec2 tex_coords = ivec2( gl_GlobalInvocationID.x, "
    "gl_GlobalInvocationID.y            ) ;\n"
    "  const vec4  color      = imageLoad( input_tex, tex_coords               "
    "                    ) ;\n"
    "  const float intensity  = ( 0.2126 * color.r + 0.7152 * color.g + 0.0722 "
    "* color.b ) * color.a ;\n"
    "  const float out_value  = intensity > config.threshold ? 1.0 : 0.0       "
    "                      ;\n"
    "  \n"
    "  vec4 out_vec = vec4( vec3( out_value ), 1.0 ) ;\n"
    "  imageStore( output_tex, tex_coords, out_vec ) ;\n"
    "}\n"};

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
  return memory.type() & HeapType::HostVisible;
}

auto test_offset() -> bool {
  constexpr auto mem_size = 2048;
  auto memory_1 = Memory<API>(0, HeapType::GpuOnly, mem_size);
  auto memory_2 = Memory<API>(memory_1, 1024);

  auto same_gpu = memory_2.gpu() == memory_1.gpu();
  auto correct_size = memory_2.size() >= 1024;
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
  auto memory_1 =
      Memory<API, PoolAllocator<API>>(0, HeapType::HostVisible, mem_size);
  auto memory_2 = Memory<API, PoolAllocator<API>>(std::move(memory_1));

  return memory_1.handle() < 0 && memory_2.handle() >= 0 &&
         memory_2.type() & HeapType::HostVisible;
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
}  // namespace array
namespace image {
auto test_creation() -> bool {
  auto image = Image<API>(0, {1280, 1024, ImageFormat::RGBA8});
  return image.gpu() == 0;
}

auto test_memory_allocation() -> bool {
  auto image = Image<API>(0, {1280, 1024, ImageFormat::RGBA8});
  const auto& memory = image.memory();
  return memory.handle() >= 0 && memory.size() > 0;
}

auto test_getters() -> bool {
  auto info = ImageInfo();
  info.width = 1280;
  info.height = 1024;
  info.layers = 2;
  info.mip_maps = 1;
  info.is_cubemap = false;

  auto image = Image<API>(0, info);

  auto pass = image.gpu() == 0 && image.format() == info.format &&
              image.width() == info.width && image.height() == info.height;

  return pass;
}
}  // namespace image
namespace render_pass {
auto test_creation() -> bool {
  auto info = RenderPassInfo();
  auto subpass = Subpass();
  subpass.attachments.push_back({});
  info.subpasses.push_back(subpass);

  auto render_pass = RenderPass<API>(0, info);
  return render_pass.handle() >= 0;
}

auto test_images() -> bool {
  auto info = RenderPassInfo();
  auto subpass = Subpass();
  subpass.attachments.push_back({});
  info.subpasses.push_back(subpass);
  auto render_pass = RenderPass<API>(0, info);
  return render_pass.images().size() == 3;
}

auto test_images_valid() -> bool {
  auto info = RenderPassInfo();
  auto subpass = Subpass();
  subpass.attachments.push_back({});
  info.subpasses.push_back(subpass);

  auto render_pass = RenderPass<API>(0, info);
  for (auto& img : render_pass.images()) {
    if (img.handle() < 0) return false;
  }
  return true;
}
}  // namespace render_pass
namespace pipeline {
auto test_creation() -> bool {
  auto pipeline =
      Pipeline<API>(0, {{{"test_shader.comp.glsl", test_compute_shader}}});
  return pipeline.handle() >= 0;
}
auto test_correct_gpu() -> bool {
  auto pipeline =
      Pipeline<API>(0, {{{"test_shader.comp.glsl", test_compute_shader}}});
  return pipeline.gpu() == 0;
}
}  // namespace pipeline
namespace descriptor {
auto test_creation() -> bool {
  auto pipeline =
      Pipeline<API>(0, {{{"test_shader.comp.glsl", test_compute_shader}}});
  auto descriptor = pipeline.descriptor();
  return descriptor.handle() >= 0;
}

auto test_binding() -> bool {
  auto pipeline =
      Pipeline<API>(0, {{{"test_shader.comp.glsl", test_compute_shader}}});
  auto image = Image<API>(0, {1024, 1024, ImageFormat::RGBA32F});
  auto descriptor = pipeline.descriptor();
  descriptor.bind("input_tex", image);
  return descriptor.handle() >= 0;
}
}  // namespace descriptor
namespace window {
auto test_creation() -> bool {
  auto window = Window<API>(0, {});
  return window.handle() >= 0;
}

auto test_images_size() -> bool {
  auto window = Window<API>(0, {});
  return !window.images().empty();
}

auto test_images_params() -> bool {
  auto window = Window<API>(0, {"lmao", 1280, 1024});
  for (auto& img : window.images()) {
    if (img.width() != 1280) return false;
    if (img.height() != 1024) return false;
  }
  return true;
}
}  // namespace window
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

  for (auto& num : host_array) {
    num = 1337;
  }

  commands.begin();
  commands.copy(host_array.data(), array);
  commands.submit();

  for (auto& num : host_array) {
    num = 0;
  }

  commands.begin();
  commands.copy(array, host_array.data());
  commands.submit();

  for (auto& num : host_array) {
    if (num != 1337) return false;
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

  for (auto& num : host_array) {
    num = 1337;
  }

  commands.begin();
  commands.copy(host_array.data(), array_host);
  commands.copy(array_host, array_1);
  commands.copy(array_host, array_2);
  commands.submit();
  commands.synchronize();

  for (auto& num : host_array) {
    num = 0;
  }

  commands.begin();
  commands.copy(host_array.data(), array_host);
  commands.copy(array_host, array_1);
  commands.copy(array_2, array_host);
  commands.submit();
  commands.synchronize();

  commands.copy(array_host, host_array.data());

  for (auto& num : host_array) {
    if (num != 1337) return false;
  }

  commands.begin();
  commands.copy(array_1, array_host);
  commands.submit();
  commands.synchronize();

  commands.copy(array_host, host_array.data());

  for (auto& num : host_array) {
    if (num != 0) return false;
  }

  return true;
}

auto test_array_to_image_copy() -> bool {
  auto array = Array<API, unsigned char>(0, 1280 * 1024 * 4);
  auto image = Image<API>(0, {1280, 1024});
  auto commands = Commands<API>(0);
  commands.begin();
  commands.copy(array, image);
  commands.submit();
  commands.synchronize();
  return true;
}

auto test_image_copy() -> bool {
  auto image_1 = Image<API>(0, {1280, 1024});
  auto image_2 = Image<API>(0, {1280, 1024});
  auto commands = Commands<API>(0);
  commands.begin();
  commands.copy(image_1, image_2);
  commands.submit();
  commands.synchronize();
  return true;
}
}  // namespace commands
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

TEST(Vulkan, Image) {
  EXPECT_TRUE(ohm::image::test_creation());
  EXPECT_TRUE(ohm::image::test_getters());
  EXPECT_TRUE(ohm::image::test_memory_allocation());
}

TEST(Vulkan, Pipeline) {
  EXPECT_TRUE(ohm::pipeline::test_creation());
  EXPECT_TRUE(ohm::pipeline::test_correct_gpu());
}

TEST(Vulkan, Descriptor) {
  EXPECT_TRUE(ohm::descriptor::test_creation());
  EXPECT_TRUE(ohm::descriptor::test_binding());
}

TEST(Vulkan, Window) {
  EXPECT_TRUE(ohm::window::test_creation());
  EXPECT_TRUE(ohm::window::test_images_size());
  EXPECT_TRUE(ohm::window::test_images_params());
}

TEST(Vulkan, RenderPass) {
  EXPECT_TRUE(ohm::render_pass::test_creation());
  EXPECT_TRUE(ohm::render_pass::test_images());
  EXPECT_TRUE(ohm::render_pass::test_images_valid());
}

TEST(Vulkan, Commands) {
  EXPECT_TRUE(ohm::commands::test_creation());
  EXPECT_TRUE(ohm::commands::test_host_to_array_copy());
  EXPECT_TRUE(ohm::commands::test_gpu_array_copy());
  EXPECT_TRUE(ohm::commands::test_array_to_image_copy());
  EXPECT_TRUE(ohm::commands::test_image_copy());
}

auto main(int argc, char* argv[]) -> int {
  ohm::System<ohm::API>::setDebugParameter("VK_LAYER_KHRONOS_validation");
  ohm::System<ohm::API>::setDebugParameter(
      "VK_LAYER_LUNARG_standard_validation");
  ohm::System<ohm::API>::initialize();
  testing::InitGoogleTest(&argc, argv);
  auto success = RUN_ALL_TESTS();
  ohm::System<ohm::API>::shutdown();
  return success;
}
