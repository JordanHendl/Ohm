#include <gtest/gtest.h>
#include <array>
#include <iostream>
#include <memory>
#include <string>
#include "ohm/io/dlloader.h"
#include "ohm/io/shader.h"

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
    "layout( binding = 3 ) buffer config\n"
    "{\n"
    "  float threshold;\n"
    "};\n"
    "void main()\n"
    "{\n"
    "  const ivec2 tex_coords = ivec2( gl_GlobalInvocationID.x, "
    "gl_GlobalInvocationID.y            ) ;\n"
    "  const vec4  color      = imageLoad( input_tex, tex_coords               "
    "                    ) ;\n"
    "  const float intensity  = ( 0.2126 * color.r + 0.7152 * color.g + 0.0722 "
    "* color.b ) * color.a ;\n"
    "  const float out_value  = intensity > threshold ? 1.0 : 0.0              "
    "                      ;\n"
    "  \n"
    "  vec4 out_vec = vec4( vec3( out_value ), 1.0 ) ;\n"
    "  imageStore( output_tex, tex_coords, out_vec ) ;\n"
    "}\n"};

std::vector<std::pair<std::string, std::string>> shaders = {
    {std::string("test.comp"), std::string(test_compute_shader)}};

namespace ohm {
namespace io {
auto test_initialization() -> bool {
  auto shader = io::Shader();
  return true;
}

auto test_compilation_from_src() -> bool {
  auto shader = io::Shader(shaders);
  return shader.stages().size() != 0;
}
}  // namespace io
}  // namespace ohm

TEST(IO, Shader) {
  EXPECT_TRUE(ohm::io::test_initialization());
  EXPECT_TRUE(ohm::io::test_compilation_from_src());
}

auto main(int argc, char* argv[]) -> int {
  testing::InitGoogleTest(&argc, argv);
  auto success = RUN_ALL_TESTS();
  return success;
}
