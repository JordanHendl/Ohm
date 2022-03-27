#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include "ohm/api/ohm.h"
#include "ohm/vulkan/vulkan_impl.h"

#include <benchmark/benchmark.h>

using API = ohm::Vulkan;
using PooledArray = ohm::Array<API, float>;

template <size_t size>
using CPUArray = std::vector<float>;

auto bench_cpu_to_cpu_transfer(benchmark::State& state) {
  const auto memory_size = state.range(0);

  auto raw_array_1 = std::vector<float>(memory_size);
  auto raw_array_2 = std::vector<float>(memory_size);

  while (state.KeepRunning()) {
    std::copy(raw_array_1.begin(), raw_array_1.end(), raw_array_2.begin());
  }
}

auto bench_cpu_to_gpu_transfer(benchmark::State& state) {
  const auto memory_size = state.range(0);

  auto host_array = PooledArray(0, memory_size, ohm::HeapType::HostVisible);
  auto raw_array = std::vector<float>(memory_size);

  auto cmds = ohm::Commands<API>(0);
  while (state.KeepRunning()) {
    cmds.copy(raw_array.data(), host_array);
    cmds.submit();
    benchmark::DoNotOptimize(host_array);
  }
}

auto bench_gpu_to_gpu_transfer(benchmark::State& state) {
  const auto memory_size = state.range(0);

  auto gpu_array_1 = PooledArray(0, memory_size, ohm::HeapType::GpuOnly);
  auto gpu_array_2 = PooledArray(0, memory_size, ohm::HeapType::GpuOnly);

  auto cmds = ohm::Commands<API>(0);
  while (state.KeepRunning()) {
    cmds.copy(gpu_array_1, gpu_array_2);
    cmds.submit();
    cmds.synchronize();
  }
}

auto bench_gpu_to_gpu_transfer_image(benchmark::State& state) {
  auto info = ohm::ImageInfo(1280, 1024, ohm::ImageFormat::RGBA8);
  auto image_1 = ohm::Image<API>(0, info);
  auto image_2 = ohm::Image<API>(0, info);
  auto cmds = ohm::Commands<API>(0);

  while (state.KeepRunning()) {
    cmds.copy(image_1, image_2);
    cmds.submit();
    cmds.synchronize();
  }
}

BENCHMARK(bench_cpu_to_cpu_transfer)->RangeMultiplier(4)->Range(1024, 524288);

BENCHMARK(bench_cpu_to_gpu_transfer)->RangeMultiplier(4)->Range(1024, 524288);

BENCHMARK(bench_gpu_to_gpu_transfer)->RangeMultiplier(4)->Range(1024, 524288);

int main(int argc, char** argv) {
  ohm::System<API>::initialize();

  benchmark::Initialize(&argc, argv);
  if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}