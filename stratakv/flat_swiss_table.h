#pragma once

#ifndef STRATAKV_FLAT_SWISS_TABLE_H
#define STRATAKV_FLAT_SWISS_TABLE_H

#include <cstdint>
#include <cstddef>

#include <thread>

#include <arm_neon.h>

#define STRATAKV_DEBUG_ALLOCATIONS

static constexpr double LOAD_FACTOR = 0.9;

using ctrl_t = std::uint8_t;

static constexpr std::size_t GROUP_WIDTH = 16;
static constexpr std::size_t CTRL_SIZE = sizeof(ctrl_t);

static constexpr std::size_t CACHE_LINE_SIZE = 128;

static constexpr ctrl_t CTRL_EMPTY = 0xFF;
static constexpr ctrl_t CTRL_DELETED = 0XFE;

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

    explicit FlatTable(std::size_t capacity, allocator_type* allocator, size_t num_shards)
        : capacity_(capacity), size_(0), num_shards_(num_shards), allocator_(allocator)
    {
        #ifdef STRATAKV_DEBUG_ALLOCATIONS
        std::cout << "FlatSwissTable Constructor: Initializing with Capacity: " << capacity << ", Num Shards: " << num_shards << std::endl;
        #endif

        size_t hot_bytes = capacity * (sizeof(key_type) + sizeof(ctrl_t));
        
        void* hot_block = allocator_->allocate(hot_bytes, 128);

        if (hot_block == nullptr) 
        {
            #ifdef STRATAKV_DEBUG_ALLOCATIONS
            std::cerr << "FlatSwissTable Hot Block Allocation Failed for " << hot_bytes << " bytes." << std::endl;
            #endif
            throw std::bad_alloc();
        }

        // Debug Macro
        #ifdef STRATAKV_DEBUG_ALLOCATIONS
        std::cout << "Capacity passed in: " << capacity << std::endl;
        std::cout << "FlatSwissTable Hot Block Allocation: " << hot_bytes << " bytes at " << hot_block << std::endl;
        #endif

        ctrl_ = static_cast<ctrl_t*>(hot_block);
        keys_ = reinterpret_cast<key_type*>(reinterpret_cast<std::byte*>(static_cast<std::byte*>(hot_block) + capacity * sizeof(ctrl_t)));

        values_ = reinterpret_cast<value_type*>(allocator_->allocate(capacity * sizeof(value_type), 128));
        if (values_ == nullptr) {
            #ifdef STRATAKV_DEBUG_ALLOCATIONS
            std::cerr << "FlatSwissTable Values Allocation Failed for " << (capacity * sizeof(value_type)) << " bytes." << std::endl;
            #endif
            throw std::bad_alloc();
        }

        #ifdef STRATAKV_DEBUG_ALLOCATIONS
        std::cout << "FlatSwissTable Values Allocation: " << (capacity * sizeof(value_type)) << " bytes at " << static_cast<void*>(values_) << std::endl;
        #endif

        #ifdef STRATAKV_DEBUG_ALLOCATIONS
            std::cout << "hot_block:        " << hot_block << " (mod 128 = " << (reinterpret_cast<uintptr_t>(hot_block) & 127) << ")\n";
            std::cout << "keys_ computed:   " << static_cast<void*>(keys_) << " (mod 128 = " << (reinterpret_cast<uintptr_t>(keys_) & 127) << ")\n";
            std::cout << "alignof(key_type) = " << alignof(key_type) << "\n";
            std::cout << "sizeof(key_type)  = " << sizeof(key_type) << "\n";
        #endif

        ::memset(ctrl_, static_cast<uint8_t>(CTRL_EMPTY), capacity * sizeof(ctrl_t));


        #ifdef STRATAKV_DEBUG_ALLOCATIONS
        std::cout << "FlatSwissTable Initialized with Capacity: " << capacity_ << ", Size: " << size_ << std::endl;
        #endif
    }

    ~FlatTable() 
    {
        // allocator_->deallocate(ctrl_, capacity_ * (sizeof(key_type) + sizeof(ctrl_t)));
        // allocator_->deallocate(values_, capacity_ * sizeof(value_type));
    }

    // Insert a key-value pair into the table
    inline bool insert(const key_type& key, const value_type& value) 
    {



        if (size_ >= static_cast<std::size_t>(capacity_ * LOAD_FACTOR)) 
        {
            // #ifdef STRATAKV_DEBUG_ALLOCATIONS
            // std::cout << "FlatSwissTable Resize Triggered: Size " << size_ << " >= Capacity " << capacity_ << " * Load Factor " << LOAD_FACTOR << std::endl;
            // #endif
            return false;
        }

        std::size_t hash = std::hash<key_type>{}(key);
        std::size_t idx = hash & (capacity_ - 1);
        ctrl_t h2 = static_cast<ctrl_t>((hash >> 57) & 0x7F);

        // #ifdef STRATAKV_DEBUG_ALLOCATIONS
        // std::cout << "Inserting Key: " << key << " with Hash: " << hash << " at Index: " << idx << " with h2: " << static_cast<int>(h2) << std::endl;
        // #endif

        while (true) 
        {
            ctrl_t c = ctrl_[idx];
            if (c == CTRL_EMPTY || c == CTRL_DELETED) {
                ctrl_[idx] = h2;
                keys_[idx] = key;
                values_[idx] = value;

                ++size_;
                return true;
            } else if (c == h2 && keys_[idx] == key) {
                // Key already exists, update value
                values_[idx] = value;
                return true;
            }
            idx = (idx + 1) & (capacity_ - 1);
        }

        return false;
    }

    inline bool find(const key_type& key, value_type& value_out)
    {
        std::size_t hash = std::hash<key_type>{}(key);
        std::size_t idx = hash & (capacity_ - 1);
        ctrl_t h2 = static_cast<ctrl_t>((hash >> 57) & 0x7F);

        while (true) {
            // SIMD instructions to get a group of ctrl bytes

            // [\0, \0,\0,\0,\0,\0,\0,\0,\0,\0,\0,\0,\0,\0,\0,\0]

            // So we should pass in the address of ctrl_[idx] to load the 16 bytes starting from there
            // Little Endian assumption for vec comparison
            
            

            ctrl_t c = ctrl_[idx];
            if (c == CTRL_EMPTY) {
                return false;
            } else if (c == h2 && keys_[idx] == key) {
                value_out = values_[idx];
                return true;
            }
            idx = (idx + 1) & (capacity_ - 1);
            __builtin_prefetch(&ctrl_[idx], 0, 1);
        }

        return false;
    }

    // TODO: Refactor this method to fix the local allocator issues and resize the table to either double or 1.5 the size of the original
    void resize()
    {
        // GLOBAL LOCKING
        if (size_ >= static_cast<size_t>(capacity_ * LOAD_FACTOR))
        {
            // Resize should double the size of the table, rehash the existing elements, then
            // set the Entries and controls equal to new Entries and controls and delete the previous
            // ones

            size_t new_size = capacity_ * 2;

            Arena arena(1 << 30); // 1 GB arena for resizing
            ArenaAllocator<std::byte> allocator(&arena);


            void* hot_block = allocator.allocate(new_size, 128);

            ctrl_t* new_ctrls = static_cast<ctrl_t*>(hot_block);
            key_type* new_keys = reinterpret_cast<key_type*>(reinterpret_cast<std::byte*>(static_cast<std::byte*>(hot_block) + new_size * sizeof(ctrl_t)));

            value_type* new_values = reinterpret_cast<value_type*>(allocator.allocate(new_size * sizeof(value_type), 128));



            std::memset(static_cast<uint8_t*>(new_ctrls), static_cast<uint8_t>(CTRL_EMPTY), new_size * sizeof(ctrl_t));

            size_t num_new_entries = 0;

            for (int i = 0; i < capacity_; ++i)
            {
                if (ctrl_[i] != CTRL_EMPTY && ctrl_[i] != CTRL_DELETED)
                {
                    key_type k = keys_[i];
                    value_type v = values_[i];

                    const size_t hash = std::hash<key_type>{}(k);

                    auto h2 = (hash >> 57) & 0X7F;

                    auto idx = hash & (new_size - 1);

                    while (true)
                    {
                        ctrl_t c = new_ctrls[idx];
                        if (c == CTRL_EMPTY || c == CTRL_DELETED)
                        {
                            new_ctrls[idx] = h2;
                            new_keys[idx] = k;
                            new_values[idx] = v;

                            ++num_new_entries;

                            break;
                        }
                        idx = (idx + 1) & (new_size - 1);
                    }


                }
            }

            allocator_->deallocate(nullptr, 0); // Dummy deallocation to match constructor

            allocator = allocator; // Update allocator to the new one used for resizing

            // Resize existing tables
            this->keys_ = new_keys;
            this->ctrl_ = new_ctrls;
            this->capacity_ = new_size;
            this->size_ = num_new_entries;
            this->values_ = new_values;
        }
    }

    inline bool erase(const key_type& key)
    {
        std::size_t hash = std::hash<key_type>{}(key);
        std::size_t idx = hash & (capacity_ - 1);
        ctrl_t h2 = static_cast<ctrl_t>((hash >> 57) & 0x7F);

        while (true) 
        {
            ctrl_t c = ctrl_[idx];
            if (c == CTRL_EMPTY) {
                return false;
            } else if (c == h2 && keys_[idx] == key) {
                ctrl_[idx] = CTRL_DELETED;
                --size_;
                return true;
            }
            idx = (idx + 1) & (capacity_ - 1);
        }

        return false;
    }

    FlatSwissTableStats stats() 
    {
        FlatSwissTableStats stats;
        stats.num_keys = size_;
        stats.num_buckets = capacity_;
        stats.num_shards = num_shards_;
        stats.capacity_per_shard = capacity_ / num_shards_;
        stats.total_capacity = capacity_;
        stats.key_size = sizeof(key_type);
        stats.value_size = sizeof(value_type);
        stats.allocator_bytes_used = allocator_->bytes_used();
        stats.allocator_total_size = allocator_->total_size();
        return stats;
    }

    inline value_type get(const key_type& key)
    {
        std::size_t hash = std::hash<key_type>{}(key);
        std::size_t idx = hash & (capacity_ - 1);
        ctrl_t h2 = static_cast<ctrl_t>((hash >> 57) & 0x7F);

        while (true) 
        {
            ctrl_t c = ctrl_[idx];
            if (c == CTRL_EMPTY) {
                return value_type{};
            } else if (c == h2 && keys_[idx] == key) {
                return values_[idx];
            }
            idx = (idx + 1) & (capacity_ - 1);
            __builtin_prefetch(&ctrl_[idx], 0, 1);
        }
        return value_type{};
    }



    private:


    allocator_type* allocator_;

    // Hot Data
    key_type* keys_     = nullptr;
    ctrl_t* ctrl_       = nullptr;

    // Cold Data
    value_type* values_ = nullptr;
    

    // Table properties
    std::size_t capacity_;
    std::size_t size_;
    std::size_t num_shards_;

};

} // namespace stratakv

#endif
