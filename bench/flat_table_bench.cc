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

static constexpr std::size_t TABLE_CAPACITY = 1 << 24; // 1M slots

class FlatSwissTableBench : public benchmark::Fixture {
public:
    void SetUp(benchmark::State& state) override {
        arena_ = std::make_unique<stratakv::Arena>(1ULL << 32); // 4 GB arena
        allocator_ = std::make_unique<stratakv::ArenaAllocator<std::byte>>(arena_.get());
        table_ = std::make_unique<stratakv::FlatTable<uint64_t, uint64_t, stratakv::ArenaAllocator<std::byte>>>(TABLE_CAPACITY, allocator_.get(), 1);

        std::mt19937 rng(42);
        for (std::size_t i = 0; i < pre_fill_count; ++i)
        {
            uint64_t k = rng();
            pre_keys_.push_back(k);
            table_->insert(k, k * 2);
        }
    }

    void TearDown(benchmark::State& state) override {
        table_.reset();
        allocator_.reset();
        arena_.reset();
        pre_keys_.clear();
    }




    std::unique_ptr<stratakv::Arena> arena_;
    std::unique_ptr<stratakv::ArenaAllocator<std::byte>> allocator_;
    std::unique_ptr<stratakv::FlatTable<uint64_t, uint64_t,stratakv::ArenaAllocator<std::byte>>> table_;
    std::vector<uint64_t> pre_keys_;
    size_t pre_fill_count = TABLE_CAPACITY / 2; 
};

BENCHMARK_DEFINE_F(FlatSwissTableBench, Insert)(benchmark::State& state) {
    // Fresh table for insert benchmark
    stratakv::Arena arena(1ULL << 30);
    stratakv::ArenaAllocator<std::byte> alloc(&arena);
    stratakv::FlatTable<uint64_t, uint64_t, stratakv::ArenaAllocator<std::byte>> table(
        1 << 24, &alloc, 1  
    );

    uint64_t i = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(table.insert(i++, i));
    }
    state.SetItemsProcessed(state.iterations());
}


BENCHMARK_DEFINE_F(FlatSwissTableBench, FindMiss)(benchmark::State& state) {
    uint64_t miss_key = 0xDEADBEEF00000000ULL;
    uint64_t val;
    for (auto _ : state) {
        benchmark::DoNotOptimize(table_->find(miss_key++, val));
    }
    state.SetItemsProcessed(state.iterations());
}
// BENCHMARK_REGISTER_F(FlatSwissTableBench, FindMiss);

BENCHMARK_DEFINE_F(FlatSwissTableBench, FindHit)(benchmark::State& state) {
    size_t idx = 0;
    uint64_t val;
    //std::shuffle(pre_keys_.begin(), pre_keys_.end(), std::mt19937{std::random_device{}()});

    for (auto _ : state) {
        benchmark::DoNotOptimize(table_->find(pre_keys_[idx++ % pre_keys_.size()], val));
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_REGISTER_F(FlatSwissTableBench, FindHit);


//BENCHMARK_REGISTER_F(FlatSwissTableBench, Insert);



BENCHMARK_MAIN();