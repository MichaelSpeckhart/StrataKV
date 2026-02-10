#pragma once

#ifndef STRATAKV_TABLE_BUCKET_H
#define STRATAKV_TABLE_BUCKET_H

#include <list>
#include <utility>

namespace stratakv {


template <typename key_type, typename value_type>
struct sequential_table_bucket {
    // No locks needed so just a std::list<std::pair<key_type, value_type>>
    std::list<std::pair<key_type, value_type>> items_;

    size_t bucket_size_;

    sequential_table_bucket() : bucket_size_(1024) {}
};


// Fine grained locking bucket - each bucket has its own mutex
template <typename key_type, typename value_type>
struct concurrent_table_bucket {
    // Need std::mutex for locking
    std::mutex bucket_mutex_;

    std::list<std::pair<key_type, value_type>> items_;

    size_t bucket_size_;

    concurrent_table_bucket() : bucket_size_(0) {}


    void acquire_lock() {
        bucket_mutex_.try_lock();
    }

    void release_lock() {
        bucket_mutex_.unlock();
    }
};


} // namespace stratakv

#endif //STRATAKV_TABLE_BUCKET_H