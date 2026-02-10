// #include <catch2/catch_test_macros.hpp>
// #include <chrono>
// #include <simple_hash_table.h>
// #include <table_bucket.h>
// #include <map.h>
// #include <cstdint>
// #include <iostream>
// #include <functional>

// #include <numa_table_entry.h>


// struct NUMATABLE
// {
//   flat_table<uint64_t, uint64_t> *flat_table;

//   NUMATABLE()
//   {
//       flat_table = new flat_table<uint64_t, uint64_t>(512);

//       for (int i = 0; i < 256; ++i)
//       {
//           flat_table->insert(i, i);
//       }


//   }

//   ~NUMATABLE()
//   {
//       delete flat_table;
//   }
// };


// TEST_CASE_METHOD(NUMATABLE, "test numa table init", "[table]")
// {
//     uint8_t val = flat_table->get_ctrl_val();

//     flat_table->insert(45, 1);

//     REQUIRE(val == CTRL_EMPTY);

//     auto isThere = flat_table->get(45);

//     std::cout << "Is there? " << isThere << std::endl;
// }
