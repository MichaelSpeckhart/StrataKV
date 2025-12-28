#include <catch2/catch_test_macros.hpp>
#include <simple_hash_table.h>
#include <cstdint>
#include <iostream>

struct TableFixture {
    SimpleHashMap<int,int> map;
    

    TableFixture() : map(64) {
        map.add(1,1);
        map.add(7, 4);
    }

    ~TableFixture() { }
};

TEST_CASE_METHOD(TableFixture, "basic get works", "[table]") {
    REQUIRE(map.get(1) == 1);
    auto value = map.get(1).value();
    auto value2 = map.get(7).value();
    std::cout << "Value: " << value2 << std::endl;
}
