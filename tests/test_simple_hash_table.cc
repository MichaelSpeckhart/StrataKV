// #include <catch2/catch_test_macros.hpp>
// #include <chrono>
// #include <simple_hash_table.h>
// #include <table_bucket.h>
// #include <map.h>
// #include <cstdint>
// #include <iostream>
// #include <functional>

// #include "striped_table.h"
// #include "storage.h"

// using namespace stratakv;

// struct TableFixture {
//     stratakv::SimpleHashMap<int,int> map;
//     stratakv::StripedTable<int,int> *striped_map;

//     Storage storage;

//     TableFixture() : map(64) {
//         striped_map = new StripedTable<int, int>(64);

//         striped_map->insert(1, 1);
//         striped_map->insert(2, 5);
//         map.add(1,1);
//         map.add(7, 4);
//         map.add(3, 5);
//         map.add(4, 6);
//     }

//     ~TableFixture() {
//         delete striped_map;
//     }
// };

// // Sequential Tests

// TEST_CASE_METHOD(TableFixture, "basic get works", "[table]") {
//     //REQUIRE(map.get(1) == 1);

//     auto value = striped_map->get(1);
//     REQUIRE(value.has_value());
//     REQUIRE(value == 1);
//     std::cout << "Value: " << value.value() << std::endl;
// }

// TEST_CASE_METHOD(TableFixture, "basic erase works", "[table]") {
//     REQUIRE(map.erase(1) == true);
//     auto value = map.get(1);

//     REQUIRE(value == std::nullopt);
// }

// TEST_CASE_METHOD(TableFixture, "basic table size", "[table]") {
//     REQUIRE(map.map_size() == 64);
// }

// TEST_CASE_METHOD(TableFixture, "basic get fails", "[table]") {
//     auto value = striped_map->get(4);

//     REQUIRE(value.has_value() == false);
// }

// TEST_CASE_METHOD(TableFixture, "basic remove works", "[table]") {
//   striped_map->erase(2);
//   auto value = striped_map->get(2);

//   REQUIRE(value.has_value() == false);

// }

// // multi-threaded tests
// #include <thread>

// inline void init_thread_pool(size_t num_threads, std::function<void(int)> func) {
//     std::vector<std::thread> pool;
//     pool.reserve(num_threads);
//     for (int i = 0; i < num_threads; ++i) {
//         pool.emplace_back(func, i);
//     }


//     for (auto& thread : pool) {
//         thread.join();
//     }
// }

// TEST_CASE_METHOD(TableFixture, "basic two threaded add", "[table]") {

//     #if defined (__MACH__)
//         printf("Hello Mac");
//     #endif


//     int total_items = 1000;

//     striped_map->erase(1);
//     striped_map->erase(2);

//     auto start = std::chrono::steady_clock::now();
//     init_thread_pool(4, [&](int thread_id) {
//         // Our "work order"
//         for (int i = thread_id * 250; i < (thread_id + 1) * 250; ++i) {
//             striped_map->insert(i, i * 10);
//         }
//     });

//     auto end = std::chrono::steady_clock::now();

//     auto time = end - start;

//     std::cout << "Time: " << time << std::endl;

//     // Verification Logic
//     int count = 0;
//     bool data_corrupted = false;

//     striped_map->do_all_readonly([&](int k, const int& v) {
//         count++;
//         if (v != k * 10) {
//             data_corrupted = true;
//         }
//     });

//     // These assertions will fail if a race condition occurred
//     REQUIRE(count == total_items);
//     REQUIRE(data_corrupted == false);
// }
