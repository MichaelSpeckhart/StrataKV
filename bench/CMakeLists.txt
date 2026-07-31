include(FetchContent)

# Fetch Google Benchmark
FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.9.1
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

# Your benchmark executable (separate from Catch2 tests)
add_executable(stratakv_bench_exec
  flat_table_bench.cc
)

add_executable(unordered_bench_exec
  unordered_map_bench.cc
)

target_link_libraries(unordered_bench_exec
  PRIVATE
    benchmark::benchmark
    benchmark::benchmark_main
    stratakv
)

target_link_libraries(stratakv_bench_exec
  PRIVATE
    benchmark::benchmark
    benchmark::benchmark_main
    stratakv
)