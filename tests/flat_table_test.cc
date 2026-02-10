#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <simple_hash_table.h>
#include <table_bucket.h>
#include <map.h>
#include <cstdint>
#include <iostream>
#include <functional>

#include "arena_allocator.h"
#include "flat_swiss_table.h"
#include <random>

#define STRATAKV_DEBUG_ALLOCATIONS


using namespace stratakv;

struct TableFixture {
    stratakv::Arena arena;
    stratakv::ArenaAllocator<std::byte> arena_allocator;
    stratakv::FlatTable<int, int, stratakv::ArenaAllocator<std::byte>> flat_table;
    
    

    TableFixture() 
        : arena(1ULL << 30), 
          arena_allocator(&arena),
          flat_table(1 << 19, &arena_allocator, 1) 
    {
        #ifdef STRATAKV_DEBUG_ALLOCATIONS
        std::cout << "TableFixture Setup: Inserting initial key-value pairs." << std::endl;
        #endif 

        flat_table.insert(1, 10);
    }

    ~TableFixture() {
    }
};

TEST_CASE_METHOD(TableFixture, "FlatTable Find Existing Key", "[FlatTable]") {
    int value;
    bool found = flat_table.find(1, value);
    REQUIRE(found);
    REQUIRE(value == 10);
}

TEST_CASE_METHOD(TableFixture, "FlatTable 1 << 18 Keys Bench", "[FlatTable]") {
    const size_t N = 1ULL << 18;

   
    std::vector<uint64_t> keys(N);
    std::mt19937_64 rng{std::random_device{}()};
    for (size_t i = 0; i < N; ++i) {
        keys[i] = rng();
    }

    
    size_t num_inserted = 0;
    for (auto k : keys) {
        if (flat_table.insert(static_cast<int>(k & 0xFFFFFFFF), static_cast<int>(k >> 32))) {
            ++num_inserted;
        }
    }
    std::cout << "Inserted " << num_inserted << " keys (expected " << N << ")" << std::endl;

    
    std::shuffle(keys.begin(), keys.end(), rng);

    
    std::cout << "Warm-up pass..." << std::endl;
    for (auto k : keys) {
        int dummy;
        flat_table.find(static_cast<int>(k & 0xFFFFFFFF), dummy);
    }

    std::cout << "Timed lookup pass..." << std::endl;
    uint64_t hits = 0;
    auto start = std::chrono::steady_clock::now();

    for (auto k : keys) {
        int value;
        if (flat_table.find(static_cast<int>(k & 0xFFFFFFFF), value)) {
            ++hits;
        }
    }

    auto end = std::chrono::steady_clock::now();
    auto ns_total = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    double ns_per_lookup = static_cast<double>(ns_total) / N;

    printf("FlatTable Lookups: %.2f ns/op (%.2f M lookups/sec)\n", ns_per_lookup, 1000.0 / ns_per_lookup);
    printf("Total time: %.3f ms\n", ns_total / 1e6);
    printf("Hits: %llu / %zu (%.2f%%)\n", hits, N, 100.0 * hits / N);

}