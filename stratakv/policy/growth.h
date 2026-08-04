//
// Created by Michael Speckhart on 8/3/26.
//

#ifndef STRATAKV_GROWTH_H
#define STRATAKV_GROWTH_H

#include <bit>
#include <cstddef>

/**
 * @growth.h
 *  defines the
 *
 *
 *
 */

struct PowerOfTwoGrowth {
    static constexpr double max_load_factor = 0.75;

    // Calculates the number of groups for power of 2 size table
    static constexpr std::size_t groups_for(const std::size_t slots_per_group, const std::size_t num_slots) {
        const std::size_t needed = (num_slots + slots_per_group - 1) / slots_per_group;
        return std::bit_ceil(needed < 1 ? 1 : needed);
    }

    static constexpr bool should_grow(std::size_t size, std::size_t capacity) {
        return size >= static_cast<std::size_t>(capacity * max_load_factor);
    }
    static constexpr std::size_t grow(std::size_t num_groups) { return num_groups * 2; }

    // the two that were duplicated 7 times
    static constexpr std::size_t index_for(std::size_t hash, std::size_t num_groups) {
        return (hash >> 7) & (num_groups - 1);
    }
    static constexpr std::size_t next_index(std::size_t idx, std::size_t num_groups) {
        return (idx + 1) & (num_groups - 1);
    }
};

#endif //STRATAKV_GROWTH_H
