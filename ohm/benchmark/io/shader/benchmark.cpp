#include <fstream>
#include <iostream>
#include <ostream>
#include "ohm/io/shader.h"

#include <benchmark/benchmark.h>

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

std::vector<std::pair<std::string, std::string>> shaders = {
    {std::string("test.comp"), std::string(test_compute_shader)}};

std::vector<std::string> shader_files = {"test_shader.comp.glsl"};
auto bench_shader_creation(benchmark::State& state) {
  while (state.KeepRunning()) {
    auto shader = ohm::io::Shader(shaders);
    benchmark::DoNotOptimize(shader);
  }
}

auto bench_shader_creation_from_file(benchmark::State& state) {
  while (state.KeepRunning()) {
    auto shader = ohm::io::Shader(shader_files);
    benchmark::DoNotOptimize(shader);
  }
}

BENCHMARK(bench_shader_creation);

int main(int argc, char** argv) {
  std::ofstream stream("test_shader.comp.glsl");
  if (stream) {
    stream.write(test_compute_shader, sizeof(test_compute_shader));
    stream.close();
  }

  benchmark::Initialize(&argc, argv);
  if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}