//
// Created by Michael Speckhart on 8/3/26.
//

#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include "internal/group128.h"

using GroupType = stratakv::Group128<uint64_t, __uint64_t>;

class Group128Fixture {
protected:

    stratakv::Group128<uint64_t, uint64_t> g_;
    using GroupType = stratakv::Group128<uint64_t, uint64_t>;
    Group128Fixture() {
        for (size_t i {}; i < GroupType::GROUP_WIDTH; ++i) {
            g_.ctrl[i] = ctrl_t::kEmpty;
        }
    }

    void fill_slots(std::size_t i, uint64_t key, uint64_t value) {
        g_.ctrl[i] = static_cast<ctrl_t>((hash_key(key)) & 0x7F);
        g_.slots[i] = {.key = key, .value = value};
    }

    static std::size_t hash_key(const uint64_t& key)
    {
        return key * 0x9e3779b97f4a7c15ULL;
    }
};

TEST_CASE_METHOD(Group128Fixture, "empty group matches all valid lanes", "[group]") {
    fill_slots(3, 42, 100);
    const auto h2 = static_cast<ctrl_t>(hash_key(42) & 0x7F);

    REQUIRE(g_.match(h2)      == (1u << 3));
    REQUIRE(g_.match_empty()  == (0x7F & ~(1u << 3)));
}

TEST_CASE_METHOD(Group128Fixture, "no matches, only empty", "[group]") {
    for (size_t i{}; i < GroupType::SLOTS_PER_GROUP; ++i) {
        fill_slots(i, i, i);
    }

    const auto h2 = static_cast<ctrl_t>(hash_key(17) & 0x7F);

    g_.match(h2);
    g_.match_empty();
    // REQUIRE(g_.match(h2)    == (0));
    // REQUIRE(g_.match_empty() == 0);

}