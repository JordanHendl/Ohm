if(Build_Benchmarks)
find_package(benchmark)
if(benchmark_FOUND)
  add_executable(benchmark_ohm_io_dlloader benchmark.cpp)
  target_link_libraries(benchmark_ohm_io_dlloader benchmark::benchmark io)
endif()
endif()
