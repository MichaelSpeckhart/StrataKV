#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <sys/types.h>
#include <type_traits>
#include <iostream>
#ifndef STRATAKV_FLAT_TABLE_H
#define STRATAKV_FLAT_TABLE_H

#include <stdint.h>

using ctrl_t = std::uint8_t;

static constexpr ctrl_t CTRL_EMPTY = 0xFF;
static constexpr ctrl_t CTRL_DELETED = 0XFE;

constexpr double LOAD_FACTOR = 0.9;

#if defined (__MACH__)

template <typename key, typename val>
requires (std::is_trivially_copyable_v<key> && std::is_trivially_copyable_v<val>
&& std::is_trivially_destructible_v<key> && std::is_trivially_destructible_v<val>)
class flat_table
{
    private:

    struct Entry
    {
        key k;
        val v;
    };

    ctrl_t* controls = nullptr;
    Entry* entries;

    size_t table_size_ = 0;

    size_t num_collisions = 0;

    size_t k = 0;

    size_t num_entries = 0;

    public:

    flat_table(size_t table_size) : table_size_(table_size)
    {
        try {
            entries = static_cast<Entry*>(std::aligned_alloc(128, sizeof(Entry) * table_size));
            controls = static_cast<ctrl_t*>(std::aligned_alloc(128, sizeof(ctrl_t) * table_size));

            std::memset(static_cast<uint8_t*>(controls), static_cast<uint8_t>(CTRL_EMPTY), table_size * sizeof(ctrl_t));

            size_t size_of_control = sizeof(ctrl_t);
            size_t key_size = sizeof(key);
            size_t val_size = sizeof(val);

            std::cout << "Controls Size: " << size_of_control << " Key Size: " << key_size << " Val Size: " << val_size << std::endl;
            std::cout << "Controls Empty: " << CTRL_EMPTY << " Controls Deleted: " << CTRL_DELETED << std::endl;

        } catch (std::exception) {
            perror("Error on initialization");
        }

        k = log2(table_size);
    }

    ctrl_t get_ctrl_val()
    {
        return controls[0];
    }

    inline bool insert(const key k, const val& v)
    {
        const size_t hash = std::hash<key>{}(k);

        auto h2 = (hash >> 57) & 0X7F;

        auto idx = hash & (table_size_ - 1);

        while (true) {
          ctrl_t c = controls[idx];
          if (c == h2 && entries[idx].k == k)
          {
              // Key found we can't insert
              return false;
          }

          if (c == CTRL_EMPTY || c == CTRL_DELETED) {
            controls[idx] = h2;
            entries[idx].k = k;
            entries[idx].v = v;

            ++num_entries;

            return true;
          }
          idx = (idx + 1) & (table_size_ - 1);
        }
        return true;
    }

    inline val get(key k, uint64_t* probe_out = nullptr)
    {
        size_t hash = std::hash<key>{}(k);
        size_t idx  = hash & (table_size_ - 1);
        ctrl_t h2   = static_cast<ctrl_t>((hash >> 57) & 0x7F);

        uint64_t probes = 0;

        while (true) {
            ++probes;

            ctrl_t c = controls[idx];
            if (c == CTRL_EMPTY) {
                if (probe_out) *probe_out += probes;
                return nullptr;
            }
            if (c == h2 && entries[idx].k == k) {
                if (probe_out) *probe_out += probes;
                return entries[idx].v;
            }
            idx = (idx + 1) & (table_size_ - 1);
        }

        return nullptr;
    }

    void resize()
    {
        // GLOBAL LOCKING
        if (num_entries >= static_cast<size_t>(table_size_ * LOAD_FACTOR))
        {
            // Resize should double the size of the table, rehash the existing elements, then
            // set the Entries and controls equal to new Entries and controls and delete the previous
            // ones

            size_t new_size = table_size_ * 2;

            Entry* new_entries = static_cast<Entry*>(std::aligned_alloc(128, sizeof(Entry) * new_size));
            ctrl_t* new_ctrls = static_cast<ctrl_t*>(std::aligned_alloc(128, sizeof(ctrl_t) * new_size));

            std::memset(static_cast<uint8_t*>(new_ctrls), static_cast<uint8_t>(CTRL_EMPTY), new_size * sizeof(ctrl_t));

            size_t num_new_entries = 0;

            for (int i = 0; i < table_size_; ++i)
            {
                if (controls[i] != CTRL_EMPTY && controls[i] != CTRL_DELETED)
                {
                    key k = entries[i].k;
                    val v = entries[i].v;

                    const size_t hash = std::hash<key>{}(k);

                    auto h2 = (hash >> 57) & 0X7F;

                    auto idx = hash & (new_size - 1);

                    while (true)
                    {
                        ctrl_t c = new_ctrls[idx];
                        if (c == CTRL_EMPTY || c == CTRL_DELETED)
                        {
                            new_ctrls[idx] = h2;
                            new_entries[idx].k = k;
                            new_entries[idx].v = v;

                            ++num_new_entries;

                            break;
                        }
                        idx = (idx + 1) & (new_size - 1);
                    }


                }
            }
            // Clear the existing tables
            std::free(entries);
            std::free(controls);

            // Resize existing tables
            this->entries = new_entries;
            this->controls = new_ctrls;
            this->table_size_ = new_size;
            this->num_entries = num_new_entries;
        }
    }



    ~flat_table()
    {
        std::free(entries);
        std::free(controls);
    }
};




#endif

#endif
