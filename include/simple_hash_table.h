#ifndef STRATADB_SIMPLE_HASH_TABLE_H
#define STRATADB_SIMPLE_HASH_TABLE_H

#pragma once


#include <vector>
#include <unordered_map>
#include <optional>

template<typename K, typename V>
class SimpleHashMap {

    private:

    std::vector<std::unordered_map<K, V>> buckets_;
    std::vector<std::mutex> locks_;
    size_t num_buckets_;

    size_t bucket_index(const K& key);

    public:

    explicit SimpleHashMap(std::size_t num_buckets = 64)
        : buckets_(num_buckets),
          locks_(num_buckets),
          num_buckets_(num_buckets) {}


    std::optional<V> get(const K& key) const {

        size_t bucket_index = (key) % num_buckets_;

        auto& bucket = buckets_.at(bucket_index);

        auto it = bucket.find(key);
        if (it == bucket.end()) return std::nullopt;
        return it->second; // returns a copy; fine for Milestone 1
    }

    bool add(const K& key, V value) {
        size_t idx = (key) % num_buckets_;

        auto& bucket = buckets_[idx];
        auto [it, inserted] = bucket.insert({key, std::move(value)});
        if (!inserted) {
            it->second = std::move(value);
        }
        return inserted;
    }

    bool erase(const K& key);
    ~SimpleHashMap() = default;

    protected:
    void acquire();
    void release();

};



#endif //STRATADB_SIMPLE_HASH_TABLE_H