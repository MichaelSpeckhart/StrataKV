//
//  flat_map_bench.cc
//
//
//  Created by Michael Speckhart on 1/13/26.
//

#include "../stratakv/numa_table_entry.h"
#include <chrono>
#include <cstdint>
#include <vector>
#include <random>
#include <cstdio>

#if defined(__OPTIMIZE__)
#warning "Compiler optimizations enabled"
#endif
#include <algorithm>

constexpr size_t N = 1 << 18;

template <typename T>
void do_not_optimize(T const& val) {
    // This tells the compiler the value is used and might be modified
    asm volatile("" : : "g"(val) : "memory");
}

int main()
{
    flat_numa_table<uint64_t, uint64_t> *table = new flat_numa_table<uint64_t, uint64_t>(N);

    printf("Size of table: %ld\n", N);

    std::vector<uint64_t> keys(N);
        for (size_t i = 0; i < N; ++i)
            keys[i] = i * 11400714819323198485ull; // good mixing

        // --------------------
        // Insert phase
        // --------------------
        size_t num_insertions = 0;
        for (auto k : keys)
        {
            num_insertions += 1;
            table->insert(k, k);
        }


        std::shuffle(keys.begin(), keys.end(), std::mt19937{std::random_device{}()});

        // --------------------
        // Warm-up (VERY important)
        // --------------------
        for (auto k : keys)
            table->get(k);

            uint64_t sum = 0;

            uint64_t total_probes = 0;
            uint64_t hits = 0;
        // --------------------
        // Timed lookup
        // --------------------
        auto start = std::chrono::steady_clock::now();



        for (auto k : keys) {
            auto res = table->get(k, &total_probes);
        }

        double probes_per_lookup = double(total_probes) / double(keys.size());
        printf("Probes/lookup: %.3f  (hits=%llu)\n",
               probes_per_lookup, (unsigned long long)hits);

        auto end = std::chrono::steady_clock::now();

        printf("Sum of vals: %lld\n", sum);

        double ns_per_lookup =
                std::chrono::duration<double, std::nano>(end - start).count() / N;

        double total_time = std::chrono::duration<double, std::nano>(end - start).count();

        printf("Lookups: %.2f ns/op\n", ns_per_lookup);
        printf("Total Operation Time: %.2f ns\n", total_time);
}
