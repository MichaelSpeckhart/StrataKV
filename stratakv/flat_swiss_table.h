#pragma once

#ifndef STRATAKV_FLAT_SWISS_TABLE_H
#define STRATAKV_FLAT_SWISS_TABLE_H

#include <cstdint>
#include <cstddef>
#include <bit>
#include <thread>
#include <iostream>

#include <arm_neon.h>

#if defined(__GNUC__) && !defined(__clang__)

#endif



static constexpr double LOAD_FACTOR = 0.75;

using ctrl_t = std::uint8_t;

static constexpr std::size_t GROUP_WIDTH = 16;
static constexpr std::size_t CTRL_SIZE = sizeof(ctrl_t);

static constexpr std::size_t CACHE_LINE_SIZE = 128;

static constexpr ctrl_t CTRL_EMPTY = 0xFF;
static constexpr ctrl_t CTRL_DELETED = 0XFE;

// Group represents 1 Cache Line
template <typename Key = uint64_t, typename Value = uint64_t>
struct alignas(128) Group128 {
    static constexpr size_t GROUP_WIDTH = 16;
    static constexpr size_t SLOTS_PER_GROUP = 7;
    uint8_t ctrl[GROUP_WIDTH];

    struct Slot {
        Key key;
        Value value;
    } slots[7];


};

static_assert(sizeof(Group128<uint64_t, uint64_t>) == 128,
              "Group must be exactly 128 bytes to fill one Apple Silicon cache line!");
static_assert(alignof(Group128<uint64_t, uint64_t>) == 128,
              "Group must be aligned to 128 bytes!");

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




template <typename key_type, typename value_type, typename allocator_type>
requires (std::is_trivially_copyable_v<key_type> && std::is_trivially_copyable_v<value_type>
&& std::is_trivially_destructible_v<key_type> && std::is_trivially_destructible_v<value_type>)
class FlatTable
{
    public:

    explicit FlatTable(const std::size_t capacity, allocator_type* allocator, const size_t num_shards)
        : allocator_(allocator), capacity_(std::bit_ceil(capacity)), size_(0), num_shards_(num_shards)
    {
        std::size_t needed_slots = static_cast<std::size_t>(capacity_ / LOAD_FACTOR) + 1;
        std::size_t needed_groups = (capacity_ + GroupType::SLOTS_PER_GROUP - 1) / GroupType::SLOTS_PER_GROUP;


        num_groups_ = std::bit_ceil(needed_groups < 1 ? 1 : needed_groups);
        capacity_   = num_groups_ * GroupType::SLOTS_PER_GROUP;

        size_t hot_bytes = capacity_ * (sizeof(ctrl_t));

        size_t total_bytes = num_groups_ * sizeof(GroupType);
        start_addr = allocator_->allocate(total_bytes);

        if (!start_addr) {
            throw std::bad_alloc();
        }

        groups_ = new (start_addr) GroupType[num_groups_];

        for (size_t g = 0; g < num_groups_; ++g) {
            ::memset(groups_[g].ctrl, static_cast<uint8_t>(CTRL_EMPTY), 16);
        }

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
        if (size_ >= static_cast<std::size_t>(capacity_ * LOAD_FACTOR)) 
        {
            resize();
        }
        __builtin_prefetch(std::addressof(groups_), 0, 1);
        const std::size_t hash  = hash_key(key);
        std::size_t group_idx   = (hash >> 7) & (num_groups_ - 1);
        const auto h2= hash & 0x7F;

        const auto initial_group = group_idx;

        while (true) {
            __builtin_prefetch(std::addressof(groups_[group_idx]));
            auto& group = groups_[group_idx];

            for (size_t i{}; i < GroupType::SLOTS_PER_GROUP; ++i) {

                const uint8_t ctrl = group.ctrl[i];

                if (ctrl == CTRL_EMPTY) {
                    group.ctrl[i] = h2;
                    group.slots[i].key = key;
                    group.slots[i].value = value;
                    size_++;
                    return true;
                }
                if (ctrl == h2 && group.slots[i].key == key) {
                    std::cout << "Key exists: " << key << "\n";
                    std::cout << "Table Size: " << size_ << "\n";
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
        std::size_t group_idx  = (hash >> 7) & (num_groups_ - 1);
        const auto h2        = hash & 0x7F;

        auto initial_group = group_idx;
        while (true)
        {
            __builtin_prefetch(std::addressof(groups_[group_idx]));
            auto& group = groups_[group_idx];


            for (size_t i{}; i < GroupType::SLOTS_PER_GROUP; ++i) {

                const uint8_t ctrl = group.ctrl[i];

                if (ctrl == CTRL_EMPTY) {
                    return false;
                }
                if (ctrl == h2 && group.slots[i].key == key) {
                    value_out = group.slots[i].value;
                    return true;
                }
            }

            group_idx = (group_idx + 1) & (num_groups_ - 1);

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
        const size_t new_num_groups    = (new_capacity + GroupType::SLOTS_PER_GROUP - 1) / GroupType::SLOTS_PER_GROUP;

        size_t total_bytes = new_num_groups * sizeof(Group128<uint64_t, uint64_t>);
        void* block = allocator_->allocate(total_bytes);

        if (!block) {
            throw std::bad_alloc();
        }

        auto* new_groups = new (block) GroupType[new_num_groups];

        for (size_t g = 0; g < new_num_groups; ++g) {
            ::memset(new_groups[g].ctrl, static_cast<uint8_t>(CTRL_EMPTY), 16);
        }

        // 3. Rehash all active entries from old groups_ into new_groups
        for (size_t i = 0; i < num_groups_; ++i) {
            auto& old_group = groups_[i];

            for (size_t j = 0; j < GroupType::SLOTS_PER_GROUP; ++j) {

                // Skip empty and deleted tombstones
                const uint8_t old_ctrl = old_group.ctrl[j];
                if (old_ctrl == CTRL_EMPTY || old_ctrl == CTRL_DELETED) {
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

                    for (size_t k = 0; k < GroupType::SLOTS_PER_GROUP; ++k) {
                        if (target_group.ctrl[k] == CTRL_EMPTY) {
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
            groups_[i].~GroupType();
        }

        size_t old_total_bytes = num_groups_ * sizeof(GroupType);
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

            for (size_t i{}; i < GroupType::SLOTS_PER_GROUP; ++i) {

                const uint8_t ctrl = group.ctrl[i];

                if (ctrl == CTRL_EMPTY) {
                    return false;
                }

                if (ctrl == CTRL_DELETED) {
                    return false;
                }
                if (ctrl == h2 && group.slots[i].key == key) {
                    group.ctrl[i] = CTRL_DELETED;
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
        //     if (c == CTRL_EMPTY) {
        //         return value_type{};
        //     } else if (c == h2 && keys_[idx] == key) {
        //         return values_[idx];
        //     }
        //     idx = (idx + 1) & (capacity_ - 1);
        // }
        // return value_type{};
    }



    private:

    inline std::size_t hash_key(const key_type& key) const
    {
        return key * 0x9e3779b97f4a7c15ULL;
    }

    inline uint64_t neon_movemask(uint8x16_t matches) const {
        uint8x8_t narrowed = vshrn_n_u16(vreinterpretq_u16_u8(matches), 4);
        return vget_lane_u64(vreinterpret_u64_u8(narrowed), 0);
    }

    using GroupType = Group128<key_type, value_type>;

    GroupType* groups_;

    allocator_type* allocator_;
    void* start_addr;

    // Table properties
    std::size_t capacity_;
    std::size_t size_;
    std::size_t num_shards_;
    std::size_t num_groups_;

};

} // namespace stratakv

#endif
