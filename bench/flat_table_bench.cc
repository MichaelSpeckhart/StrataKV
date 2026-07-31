//
//  flat_table_bench.cc
//  
//
//  Created by Michael Speckhart on 2/10/26.
//

#include <benchmark/benchmark.h>

#include "arena_allocator.h"
#include "flat_swiss_table.h"

#include <random>
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace {
    class FlatSwissTableBench : public benchmark::Fixture {
    public:
        void SetUp(benchmark::State& state) override {
            // Read capacity dynamically from parameter range (e.g., 1<<16, 1<<20, 1<<24)
            size_t capacity = state.range(0);
            size_t pre_fill_count = static_cast<size_t>(capacity * 0.75);

            // 4 GB arena to comfortably support large table allocations
            arena_ = std::make_unique<stratakv::Arena>(1ULL << 32);
            allocator_ = std::make_unique<stratakv::ArenaAllocator<std::byte>>(arena_.get());
            table_ = std::make_unique<stratakv::FlatTable<uint64_t, uint64_t, stratakv::ArenaAllocator<std::byte>>>(
                capacity, allocator_.get(), 1
            );

            pre_keys_.reserve(pre_fill_count);
            std::mt19937_64 rng(42); // Deterministic 64-bit generator

            for (std::size_t i = 0; i < pre_fill_count; ++i) {
                uint64_t k = rng();
                pre_keys_.push_back(k);
                table_->insert(k, k * 2);
            }

            // Shuffle keys deterministically once before benchmark starts
            std::shuffle(pre_keys_.begin(), pre_keys_.end(), rng);
        }

        void TearDown(benchmark::State& state) override {
            table_.reset();
            allocator_.reset();
            arena_.reset();
            pre_keys_.clear();
        }

        std::unique_ptr<stratakv::Arena> arena_;
        std::unique_ptr<stratakv::ArenaAllocator<std::byte>> allocator_;
        std::unique_ptr<stratakv::FlatTable<uint64_t, uint64_t, stratakv::ArenaAllocator<std::byte>>> table_;
        std::vector<uint64_t> pre_keys_;
    };
}

// 1. FindHit Benchmark
BENCHMARK_DEFINE_F(FlatSwissTableBench, FindHit)(benchmark::State& state) {
    const size_t num_keys = pre_keys_.size();
    size_t idx = 0;
    uint64_t val = 0;

    for (auto _ : state) {
        uint64_t key = pre_keys_[idx];
        bool found = table_->find(key, val);

        benchmark::DoNotOptimize(found);
        benchmark::DoNotOptimize(val);

        idx = (idx + 1 == num_keys) ? 0 : idx + 1;
    }
    state.SetItemsProcessed(state.iterations());
}

// 2. FindMiss Benchmark
BENCHMARK_DEFINE_F(FlatSwissTableBench, FindMiss)(benchmark::State& state) {
    uint64_t miss_key = 1; // Sequential odd numbers to guarantee miss vs RNG keys
    uint64_t val = 0;

    for (auto _ : state) {
        bool found = table_->find(miss_key, val);

        benchmark::DoNotOptimize(found);
        benchmark::DoNotOptimize(val);

        miss_key += 2;
    }
    state.SetItemsProcessed(state.iterations());
}

// 3. Insert Benchmark
BENCHMARK_DEFINE_F(FlatSwissTableBench, Insert)(benchmark::State& state) {
    const size_t capacity = state.range(0);
    uint64_t key = 0;

    for (auto _ : state) {
        state.PauseTiming(); // Don't time arena or table creation overhead
        stratakv::Arena arena(1ULL << 30);
        stratakv::ArenaAllocator<std::byte> alloc(&arena);
        stratakv::FlatTable<uint64_t, uint64_t, stratakv::ArenaAllocator<std::byte>> table(
            capacity, &alloc, 1
        );
        state.ResumeTiming();

        // Insert up to 75% load factor
        // size_t target_inserts = static_cast<size_t>(capacity * 0.75);
        // for (size_t i = 0; i < target_inserts; ++i) {
        //     bool inserted = table.insert(key++, key * 2);
        //     benchmark::DoNotOptimize(inserted);
        // }
    }
    state.SetItemsProcessed(state.iterations() * static_cast<size_t>(capacity * 0.75));
}

// Register Benchmarks with range multiplier matching the unordered_map test
BENCHMARK_REGISTER_F(FlatSwissTableBench, FindHit)
->RangeMultiplier(16)
     ->Range(1 << 16, 1 << 24)
     ->Unit(benchmark::kNanosecond);

// BENCHMARK_REGISTER_F(FlatSwissTableBench, FindMiss)
//     ->RangeMultiplier(16)
//     ->Range(1 << 16, 1 << 24)
//     ->Unit(benchmark::kNanosecond);
//
// BENCHMARK_REGISTER_F(FlatSwissTableBench, Insert)
//     ->RangeMultiplier(16)
//     ->Range(1 << 16, 1 << 24)
//     ->Unit(benchmark::kNanosecond);

BENCHMARK_MAIN();