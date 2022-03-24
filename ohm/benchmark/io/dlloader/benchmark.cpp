#include <benchmark/benchmark.h>
#include "ohm/io/dlloader.h"

auto bench_dll_load(benchmark::State& state) {
  while (state.KeepRunning()) {
  }
}

BENCHMARK(bench_dll_load);

int main(int argc, char** argv) {
  benchmark::Initialize(&argc, argv);
  if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}