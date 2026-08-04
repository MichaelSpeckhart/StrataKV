#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <simple_hash_table.h>
#include <../stratakv/misc/table_bucket.h>
#include <../stratakv/misc/map.h>
#include <cstdint>
#include <iostream>
#include <functional>
#include <utility>
#include <algorithm>
#include "../stratakv/memory/arena_allocator.h"
#include "../stratakv/container/flat_swiss_table.h"
#include <random>

#define STRATAKV_DEBUG_ALLOCATIONS

template <typename T>
inline void do_not_optimize(T&& value) {
    // 'r' puts value in a register; 'g' accepts registers, memory, or immediate
    asm volatile("" : : "g"(value) : "memory");
}

using namespace stratakv;

struct UnorderedMapFixture {

    std::unordered_map<int, int> flat_table;

    UnorderedMapFixture()
        : flat_table(1 << 19)
    {
        #ifdef STRATAKV_DEBUG_ALLOCATIONS
        std::cout << "TableFixture Setup: Inserting initial key-value pairs." << std::endl;
        #endif

        flat_table.insert({1, 10});
    }

    ~UnorderedMapFixture() = default;


};

struct TableFixture {
    stratakv::Arena arena;
    stratakv::ArenaAllocator<std::byte> arena_allocator;
    stratakv::FlatTable<int64_t, int64_t, stratakv::ArenaAllocator<std::byte>> flat_table;
    
    

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

    ~TableFixture() = default;
};

static constexpr uint64_t mix64(uint64_t x) {
    x ^= x >> 30; x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27; x *= 0x94d049bb133111ebULL;
    x ^= x >> 31;
    return x;
}

TEST_CASE_METHOD(TableFixture, "FlatTable Find Existing Key", "[FlatTable]") {
    int64_t value;
    bool found = flat_table.find(1, value);
    REQUIRE(found);
    REQUIRE(value == 10);
}


TEST_CASE_METHOD(TableFixture, "FlatTable Find Missing Key", "[FlatTable]") {
    int64_t value;
    bool found = flat_table.find(2, value);
    REQUIRE(!found);
    std::cout << "Value: " << value << "\n";
}

TEST_CASE_METHOD(TableFixture, "FlatTable std::string Values", "[FlatTable]") {
    std::string value{};

    Group128<std::string, std::string> g;

    std::cout << sizeof(g) << "\n";

}

TEST_CASE_METHOD(TableFixture, "FlatTable erase ", "[FlatTable]") {
    int64_t key{};

    REQUIRE(flat_table.insert(12, 40));

    key = 12;

    REQUIRE(flat_table.erase(key));
    REQUIRE(!flat_table.erase(key));
}




TEST_CASE_METHOD(TableFixture, "FlatTable 1 << 18 Keys Bench", "[FlatTable]") {
    const size_t N = 1ULL << 18;

   
    std::vector<uint64_t> keys(N);
    std::mt19937_64 rng{std::random_device{}()};
    for (size_t i = 0; i < N; ++i) {
        keys[i] = rng();
    }

    std::cout << "Starting Key Insertion\n";
    size_t num_inserted = 0;
    for (int i = 0; i < N; ++i) {
        if (flat_table.insert(static_cast<int64_t>(mix64(i)), static_cast<int64_t>(i >> 32))) {
            ++num_inserted;
        } else {
            break;
        }
    }
    std::cout << "Inserted " << num_inserted << " keys (expected " << N << ")" << std::endl;
    REQUIRE(num_inserted == keys.size());
    
    std::ranges::shuffle(keys, rng);

    
    std::cout << "Warm-up pass..." << std::endl;
    for (int i = 0; i < N; ++i) {
        int64_t dummy;
        flat_table.find(static_cast<int64_t>(mix64(i)), dummy);
    }

    std::cout << "Timed lookup pass..." << std::endl;
    uint64_t hits = 0;
    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < N; ++i) {
        int64_t value;
        if (flat_table.find(static_cast<int64_t>(mix64(i)), value)) {
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

TEST_CASE_METHOD(UnorderedMapFixture, "UnorderedMap 1 << 18 Keys Bench", "[FlatTable]") {
    const size_t N = 1ULL << 18;


    std::vector<uint64_t> keys(N);
    std::mt19937_64 rng{std::random_device{}()};
    for (size_t i = 0; i < N; ++i) {
        keys[i] = rng();
    }


    size_t num_inserted = 0;
    for (auto k : keys) {
        if (flat_table.insert({static_cast<int64_t>(k & 0xFFFFFFFF), static_cast<int64_t>(k >> 32)}).second) {
            ++num_inserted;
        }
    }
    std::cout << "Inserted " << num_inserted << " keys (expected " << N << ")" << std::endl;


    std::shuffle(keys.begin(), keys.end(), rng);


    std::cout << "Warm-up pass..." << std::endl;
    for (auto k : keys) {
        bool isThere = flat_table.contains(static_cast<int64_t>(k & 0xFFFFFFFF));
        do_not_optimize(isThere);
    }

    std::cout << "Timed lookup pass..." << std::endl;
    uint64_t hits = 0;
    auto start = std::chrono::steady_clock::now();

    for (auto k : keys) {
        int64_t value;
        if (flat_table.contains(static_cast<int64_t>(k & 0xFFFFFFFF))) {
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