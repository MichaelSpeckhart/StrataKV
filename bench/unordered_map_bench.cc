//
//  unordered_map_bench.cc
//
#include <benchmark/benchmark.h>
#include <random>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <memory>

class UnorderedMapBench : public benchmark::Fixture {
public:
    void SetUp(benchmark::State& state) override {
        // Use state.range(0) so we can run tests across 2^16, 2^20, 2^24 easily!
        size_t capacity = state.range(0);
        size_t pre_fill_count = static_cast<size_t>(capacity * 0.75);

        table_ = std::make_unique<std::unordered_map<uint64_t, uint64_t>>();
        table_->reserve(capacity);
        pre_keys_.reserve(pre_fill_count);

        std::mt19937_64 rng(42); // Use 64-bit RNG for 64-bit keys directly
        for (std::size_t i = 0; i < pre_fill_count; ++i) {
            uint64_t k = rng();
            pre_keys_.push_back(k);
            table_->insert({k, k * 2});
        }
    }

    void TearDown(benchmark::State& state) override {
        table_.reset();
        pre_keys_.clear();
    }

    std::unique_ptr<std::unordered_map<uint64_t, uint64_t>> table_;
    std::vector<uint64_t> pre_keys_;
};

// 1. FindHit
BENCHMARK_DEFINE_F(UnorderedMapBench, FindHit)(benchmark::State& state) {
    const size_t num_keys = pre_keys_.size();
    size_t idx = 0;

    for (auto _ : state) {
        uint64_t key = pre_keys_[idx];
        auto it = table_->find(key);
        benchmark::DoNotOptimize(it);

        idx = (idx + 1 == num_keys) ? 0 : idx + 1;
    }
    state.SetItemsProcessed(state.iterations());
}

// 2. FindMiss
BENCHMARK_DEFINE_F(UnorderedMapBench, FindMiss)(benchmark::State& state) {
    // Generate sequential odd keys guaranteed to miss 64-bit RNG pre-filled keys
    uint64_t miss_key = 1;

    for (auto _ : state) {
        auto it = table_->find(miss_key);
        benchmark::DoNotOptimize(it);
        miss_key += 2;
    }
    state.SetItemsProcessed(state.iterations());
}

// Register benchmarks across sizes: 2^16, 2^20, 2^24
BENCHMARK_REGISTER_F(UnorderedMapBench, FindHit)
    ->RangeMultiplier(16)
    ->Range(1 << 16, 1 << 24)
    ->Unit(benchmark::kNanosecond);

// BENCHMARK_REGISTER_F(UnorderedMapBench, FindMiss)
//     ->RangeMultiplier(16)
//     ->Range(1 << 16, 1 << 24)
//     ->Unit(benchmark::kNanosecond);

BENCHMARK_MAIN();