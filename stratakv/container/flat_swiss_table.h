#pragma once

#ifndef STRATAKV_FLAT_SWISS_TABLE_H
#define STRATAKV_FLAT_SWISS_TABLE_H

#include <cstdint>
#include <cstddef>
#include <bit>
#include <thread>
#include <iostream>


#include "../internal/group128.h"
#include "policy/growth.h"
#include "internal/control_bytes.h"

#include <arm_neon.h>

#include "policy/group.h"

#if defined(__GNUC__) && !defined(__clang__)

#endif

inline int8_t H2(size_t hash) { return hash >> (sizeof(size_t) * 8 - 7); }

static constexpr std::size_t next_power_of_two(std::size_t n) {
    if (n == 0) return 1;
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    return n + 1;
}

namespace stratakv {

struct alignas(128) FlatSwissTableStats
{
    std::size_t num_keys;
    std::size_t num_buckets;
    std::size_t num_shards;
    std::size_t capacity_per_shard;
    std::size_t total_capacity;
    std::size_t key_size;
    std::size_t value_size;
    std::size_t allocator_bytes_used;
    std::size_t allocator_total_size;
};

struct alignas(128) FlatSwissTableShardStats
{
    std::size_t shard_index;
    std::size_t num_keys;
    std::size_t capacity;
    std::size_t allocator_bytes_used;
    std::size_t allocator_total_size;
};

template <class Policy, class... Params>
class BaseFlatTable {

};


// Growth Policy

template <typename key_type,
    typename value_type,
    typename allocator_type,
    typename group_policy = Group128<key_type, value_type>,
    typename growth_policy = PowerOfTwoGrowth>
requires (std::is_trivially_copyable_v<key_type> && std::is_trivially_copyable_v<value_type>
&& std::is_trivially_destructible_v<key_type> && std::is_trivially_destructible_v<value_type>)
class FlatTable
{
    public:

    explicit FlatTable(const std::size_t capacity, allocator_type* allocator, const size_t num_shards)
        : allocator_(allocator), capacity_(std::bit_ceil(capacity)), size_(0), num_shards_(num_shards)
    {
        num_groups_ = growth_policy::groups_for(group_policy::SLOTS_PER_GROUP, capacity_);
        capacity_   = num_groups_ * group_policy::SLOTS_PER_GROUP;

        // begin: thrown into allocator policy
        size_t hot_bytes = capacity_ * (sizeof(ctrl_t));

        size_t total_bytes = num_groups_ * sizeof(group_policy);
        start_addr = allocator_->allocate(total_bytes);

        if (!start_addr) {
            throw std::bad_alloc();
        }

        groups_ = new (start_addr) group_policy[num_groups_];

        for (size_t g = 0; g < num_groups_; ++g) {
            ::memset(groups_[g].ctrl, static_cast<uint8_t>(ctrl_t::kEmpty), 16);
        }
        // end

        this->size_ = 0;
    }

    ~FlatTable() = default;


    template <class... Args>
    bool emplace(Args&&... args)
    {



        return false;
    }

    // Insert a key-value pair into the table
    inline bool insert(const key_type& key, const value_type& value)
    {
        if (size_ >= static_cast<std::size_t>(capacity_ * growth_policy::max_load_factor))
        {
            resize();
        }
        __builtin_prefetch(__builtin_addressof(groups_), 0, 1);
        const std::size_t hash  = hash_key(key);
        std::size_t group_idx   = (hash >> 7) & (num_groups_ - 1);
        const auto h2 = H2(hash);

        const auto initial_group = group_idx;

        while (true) {
            __builtin_prefetch(__builtin_addressof(groups_[group_idx]));
            auto& group = groups_[group_idx];

            group.scalar_match(h2);

            for (size_t i{}; i < group_policy::SLOTS_PER_GROUP; ++i) {

                const ctrl_t ctrl = group.ctrl[i];

                if (IsEmpty(ctrl)) {
                    group.ctrl[i] = h2;
                    group.slots[i].key = key;
                    group.slots[i].value = value;
                    size_++;
                    return true;
                }
                if (ctrl == static_cast<ctrl_t>(h2) && group.slots[i].key == key) {
                    return false;
                }
            }

            group_idx = (group_idx + 1) & (num_groups_ - 1);

            if (group_idx == initial_group) {
                return false;
            }

        }

        return false;
    }


    inline bool find(const key_type& key, value_type& value_out)
    {
        __builtin_prefetch(std::addressof(groups_), 0, 1);
        const std::size_t hash = hash_key(key);
        std::size_t group_idx  = growth_policy::index_for(hash, num_groups_);
        const auto h2        = H2(hash);

        auto initial_group = group_idx;
        while (true)
        {
            __builtin_prefetch(std::addressof(groups_[group_idx]));
            auto& group = groups_[group_idx];

            auto res = group.match(h2);

            if (group.ctrl[std::countr_zero(res)] == h2 &&
                group.slots[std::countr_zero(res).key == key]) {
                return true;
            }

            if (res == 0) {
                return false;
            }

            group_idx = growth_policy::next_index(group_idx, num_groups_);

            __builtin_prefetch(&groups_[group_idx]);

            if (group_idx == initial_group) {
                return false;
            }
        }

        return false;
    }

    // TODO: Refactor to use Group-based layout
    void resize()
    {
        const size_t new_capacity  = capacity_ * 2;
        const size_t new_num_groups    = (new_capacity + group_policy::SLOTS_PER_GROUP - 1) / group_policy::SLOTS_PER_GROUP;

        size_t total_bytes = new_num_groups * sizeof(Group128<uint64_t, uint64_t>);
        void* block = allocator_->allocate(total_bytes);

        if (!block) {
            throw std::bad_alloc();
        }

        auto* new_groups = new (block) group_policy[new_num_groups];

        for (size_t g = 0; g < new_num_groups; ++g) {
            ::memset(new_groups[g].ctrl, static_cast<uint8_t>(ctrl_t::kEmpty), 16);
        }

        // 3. Rehash all active entries from old groups_ into new_groups
        for (size_t i = 0; i < num_groups_; ++i) {
            auto& old_group = groups_[i];

            for (size_t j = 0; j < group_policy::SLOTS_PER_GROUP; ++j) {

                // Skip empty and deleted tombstones
                const auto old_ctrl = old_group.ctrl[j];
                if (old_ctrl == ctrl_t::kEmpty || old_ctrl == ctrl_t::kDeleted) {
                    continue;
                }

                const auto& key   = old_group.slots[j].key;
                const auto& value = old_group.slots[j].value;

                // Compute hash for insertion into new_groups
                const std::size_t hash = hash_key(key);
                std::size_t group_idx  = (hash >> 7) & (new_num_groups - 1);
                const uint8_t h2       = static_cast<uint8_t>(hash & 0x7F);

                // Linear probe across new_groups to place the element
                bool inserted = false;
                while (!inserted) {
                    auto& target_group = new_groups[group_idx];

                    for (size_t k = 0; k < group_policy::SLOTS_PER_GROUP; ++k) {
                        if (target_group.ctrl[k] == ctrl_t::kEmpty) {
                            target_group.ctrl[k]   = h2;
                            target_group.slots[k].key   = key;
                            target_group.slots[k].value = value;
                            inserted = true;
                            break; // Exit slot loop
                        }
                    }

                    if (!inserted) {
                        group_idx = (group_idx + 1) % new_num_groups; // Advance to next group
                    }
                }
            }
        }

        for (size_t i = 0; i < num_groups_; ++i) {
            groups_[i].~group_policy();
        }

        size_t old_total_bytes = num_groups_ * sizeof(group_policy);
        allocator_->deallocate(static_cast<std::byte*>(start_addr), old_total_bytes);

        this->groups_ = new_groups;
        this->num_groups_ = new_num_groups;
        this->start_addr = block;

    }


    inline bool erase(const key_type& key)
    {
        const std::size_t hash  = hash_key(key);
        std::size_t group_idx   = (hash >> 7) & (num_groups_ - 1);
        const auto h2= hash & 0x7F;

        const auto initial_group = group_idx;

        while (true) {
            auto& group = groups_[group_idx];

            for (size_t i{}; i < group_policy::SLOTS_PER_GROUP; ++i) {

                const auto ctrl = group.ctrl[i];

                if (ctrl == ctrl_t::kEmpty) {
                    return false;
                }

                if (ctrl == ctrl_t::kDeleted) {
                    return false;
                }
                if (ctrl == h2 && group.slots[i].key == key) {
                    group.ctrl[i] = ctrl_t::kDeleted;
                    return true;
                }
            }

            group_idx = (group_idx + 1) & (num_groups_ - 1);
            __builtin_prefetch(std::addressof(groups_[group_idx]));
        }
    }

    FlatSwissTableStats stats()
    {
        FlatSwissTableStats stats;
        stats.num_keys            = size_;
        stats.num_buckets         = capacity_;
        stats.num_shards          = num_shards_;
        stats.capacity_per_shard  = capacity_ / num_shards_;
        stats.total_capacity      = capacity_;
        stats.key_size            = sizeof(key_type);
        stats.value_size          = sizeof(value_type);
        stats.allocator_bytes_used  = allocator_->bytes_used();
        stats.allocator_total_size  = allocator_->total_size();
        return stats;
    }

    inline value_type get(const key_type& key)
    {
        return value_type{};
        // std::size_t hash = hash_key(key);
        // std::size_t idx  = hash & (capacity_ - 1);
        // ctrl_t h2        = static_cast<ctrl_t>((hash >> 57) & 0x7F);
        //
        // while (true)
        // {
        //     ctrl_t c = ctrl_[idx];
        //     if (c == ctrl_t::kEmpty) {
        //         return value_type{};
        //     } else if (c == h2 && keys_[idx] == key) {
        //         return values_[idx];
        //     }
        //     idx = (idx + 1) & (capacity_ - 1);
        // }
        // return value_type{};
    }



    private:

    // Hash Policy
    inline std::size_t hash_key(const key_type& key) const
    {
        return key * 0x9e3779b97f4a7c15ULL;
    }

    // neon Policy
    inline uint64_t neon_movemask(uint8x16_t matches) const {
        uint8x8_t narrowed = vshrn_n_u16(vreinterpretq_u16_u8(matches), 4);
        return vget_lane_u64(vreinterpret_u64_u8(narrowed), 0);
    }


    group_policy* groups_;

    allocator_type* allocator_;
    void* start_addr;

    // Table properties
    std::size_t capacity_;
    std::size_t size_;
    std::size_t num_shards_;
    std::size_t num_groups_;
    std::size_t tombstones_;

};

} // namespace stratakv

#endif

// Need to define SlotPolicy ->
// define HashPolicy
// define
