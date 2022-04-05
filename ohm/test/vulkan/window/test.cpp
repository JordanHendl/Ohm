#include <gtest/gtest.h>
#include <array>
#include <iostream>
#include <memory>
#include "ohm/api/ohm.h"
#include "ohm/vulkan/vulkan_impl.h"

const char* test_shader_sequence = {
    "#version 450 core\n"
    "#extension GL_ARB_separate_shader_objects : enable\n"
    "#extension GL_GOOGLE_include_directive    : enable\n"
    "\n"
    "#define BLOCK_SIZE_X 32\n"
    "#define BLOCK_SIZE_Y 32\n"
    "#define BLOCK_SIZE_Z 1\n"
    "\n"
    "layout(local_size_x = BLOCK_SIZE_X, local_size_y = BLOCK_SIZE_Y, "
    "local_size_z = BLOCK_SIZE_Z) in;\n"
    "layout(binding = 0, rgba32f) coherent restrict uniform image2D c_output;\n"
    "\n"
    "void main()\n"
    "{\n"
    "  ivec2 tex_coords = ivec2(gl_GlobalInvocationID.x, "
    "gl_GlobalInvocationID.y);\n"
    "  vec4 color;\n"
    "  ivec2 resolution = imageSize(c_output);\n"
    "  const int n_bars = 12;\n"
    "  const int bar_width = resolution.x / n_bars;\n"
    "  // blocks 0, 1, 2, 3, ...  n_bars - 1;\n"
    "  int quotient = tex_coords.x / bar_width;\n"
    "  color.rgb = vec3(float(quotient) / float(n_bars-1));\n"
    "  color.a = 1.0f ;\n"
    "  if (tex_coords.y > (resolution.y/2)) {\n"
    "    color.rgb = vec3(1.0f) - color.rgb ;\n"
    "  }\n"
    "\n"
    "  imageStore(c_output, tex_coords, color);\n"
    "}\n"};

static auto running = true;

namespace ohm {
using API = ohm::Vulkan;
auto callback(const Event& event) {
  if(event.type() == Event::Type::WindowExit) {
    running = false;
  }
}

auto test() -> bool {
  auto window = Window<API>(0, {"Test Window", 1280, 1024});
  auto pipeline =
      Pipeline<API>(0, {{{"test_sequence.comp.glsl", test_shader_sequence}}});
  auto descriptor = pipeline.descriptor();
  auto image = Image<API>(0, {1280, 1024, ImageFormat::RGBA32F});
  auto cmd = Commands<API>(0);
  auto cb = EventRegister<API>();
  auto x = 1280 / 32;
  auto y = 1024 / 32;

  descriptor.bind("c_output", image);
  cmd.begin();
  cmd.bind(descriptor);
  cmd.dispatch(x, y);
  cmd.blit(image, window);
  window.wait(cmd);
  cb.add(&callback);
  while (running) {
    poll_events<API>();
    window.present();
  }
  return true;
}
}  // namespace ohm

TEST(Vulkan, Window) { EXPECT_TRUE(ohm::test()); }

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
