#ifndef STRATAKV_SIMPLE_HASH_TABLE_H
#define STRATAKV_SIMPLE_HASH_TABLE_H

#pragma once


#include <vector>
#include <unordered_map>
#include <optional>

namespace stratakv {

template <typename K, typename V>
class SimpleHashMap {

    private:

    std::vector<std::unordered_map<K, V>> buckets_;
    mutable std::vector<std::mutex> locks_;
    size_t num_buckets_;

    size_t bucket_index(const K& key);

    public:

    explicit SimpleHashMap(std::size_t num_buckets = 64)
        : buckets_(num_buckets),
          locks_(num_buckets),
          num_buckets_(num_buckets) {}

    bool contains(const K& key) const {
        auto idx = std::hash<K>{}(key) % num_buckets_;

        std::lock_guard<std::mutex> guard(locks_[idx]);
        auto& bucket = buckets_.at(idx);

        return bucket.find(key) != bucket.end();
    }


    std::optional<V> get(const K& key) const {


        auto idx = std::hash<K>{}(key) % num_buckets_;

        std::lock_guard<std::mutex> guard(locks_[idx]);
        auto& bucket = buckets_.at(idx);

        auto it = bucket.find(key);
        if (it == bucket.end()) return std::nullopt;
        return it->second; 
        // End lock
    }

    bool add(const K& key, V value) {
        size_t idx = (key) % num_buckets_;
        auto& bucket = buckets_[idx];

        

        std::lock_guard<std::mutex> guard(locks_[idx]);
        

        auto [it, inserted] = bucket.insert({key, std::move(value)});
        if (!inserted) {
            it->second = std::move(value);
        }

        return inserted;
    }

    bool erase(const K& key) {
        size_t idx = (key) % num_buckets_;

        auto& bucket = buckets_.at(idx);

        auto it = bucket.find(key);

        if (it == bucket.end()) return false;

        bucket.erase(key);


        return true;
    }

    size_t map_size() { return num_buckets_; }

    ~SimpleHashMap() = default;

    protected:
    void acquire();
    void release();

};

} // namespace stratakv



#endif //STRATADB_SIMPLE_HASH_TABLE_H