#include <iostream>
#include "api/bits/pool_allocator.h"
#include "api/ohm.h"
#include "vulkan/vulkan_impl.h"

#include <benchmark/benchmark.h>

using API = ohm::Vulkan;
auto bench_device_creation(benchmark::State& state) {
  while (state.KeepRunning()) {
    auto devices = ohm::System<API>::devices();
    benchmark::DoNotOptimize(devices);
  }
}

auto bench_memory_allocation_from_gpu(benchmark::State& state) {
  const auto memory_size = state.range(0);
  while (state.KeepRunning()) {
    auto memory = ohm::Memory<API>(0, ohm::HeapType::GpuOnly, memory_size);
    benchmark::DoNotOptimize(memory);
  }
}

auto bench_memory_allocation_from_pool(benchmark::State& state) {
  const auto memory_size = state.range(0);
  while (state.KeepRunning()) {
    auto memory = ohm::Memory<API, ohm::PoolAllocator<API>>(0, memory_size);
    benchmark::DoNotOptimize(memory);
  }
}

BENCHMARK(bench_device_creation);
BENCHMARK(bench_memory_allocation_from_gpu)
    ->RangeMultiplier(4)
    ->Range(1024, 524288);
BENCHMARK(bench_memory_allocation_from_pool)
    ->RangeMultiplier(4)
    ->Range(1024, 524288);

int main(int argc, char** argv) {
  ohm::System<API>::initialize();

  benchmark::Initialize(&argc, argv);
  if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}