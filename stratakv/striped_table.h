// #pragma once

// #ifndef STRATAKV_STRIPED_TABLE_H
// #define STRATAKV_STRIPED_TABLE_H


// #include "map.h"

// namespace stratakv {

//     template <typename key_type, typename value_type>
//     class StripedTable : public Map<key_type, value_type> {

//         struct bucket_t {
//             std::list<std::pair<key_type, value_type>> items_;
//         };

//         public:

//         StripedTable() : map_size_(64), locks_(64) {
//             for (int i = 0; i < 64; ++i) {
//                 map_.push_back(new bucket_t());
//             }
//         }

//         StripedTable(size_t num_buckets) : map_size_(num_buckets), locks_(num_buckets) {
//             for (int i = 0; i < num_buckets; ++i) {
//                 map_.push_back(new bucket_t());
//             }
//         }

//         bool insert(const key_type& key, const value_type& value) {
//             auto idx = key % map_size_;

//             std::lock_guard<std::mutex> guard(locks_[idx]);

//             map_[idx]->items_.push_front({key, value});
//             return true;

//         }

//         bool contains(const key_type& key) const {
//             auto idx = std::hash<key_type>{}(key) % map_size_;

//             auto *bucket = map_.at(idx);

//             for (auto& iter : bucket->items_) {
//                 if (iter.first == key) {
//                     return true;
//                 }
//             }

//             return false;
//         }

//         bool erase(const key_type& key) {
//             auto idx = std::hash<key_type>{}(key) % map_size_;

//             bucket_t *bucket = map_.at(idx);

//             for (auto it = bucket->items_.begin(); it != bucket->items_.end(); ++it) {
//                 if (it->first == key) {
//                     bucket->items_.erase(it);
//                     return true;
//                 }
//             }

//             return false;

//         }

//         std::optional<value_type> get(const key_type& key) const {
//             auto idx = std::hash<key_type>{}(key) % map_size_;

//             bucket_t *bucket = map_.at(idx);

//             for (auto& iter : bucket->items_) {
//                 if (iter.first == key) {
//                     return iter.second;
//                 }
//             }

//             return {};
//         }

//         // Simple function to perform a function on every key-value pair in map
//         // for read-only purposes
//         // Update the signature to accept the key and value
//         inline void do_all_readonly(std::function<void(const key_type&, const value_type&)> func) const {
//             for (size_t i = 0; i < map_size_; ++i) {
//                 std::lock_guard<std::mutex> guard(locks_[i]);
//                 for (const auto& pair : map_[i]->items_) {
//                     func(pair.first, pair.second);
//                 }
//             }
//         }

//         size_t map_size() const {
//             return map_size_;
//         }

//         ~StripedTable() {
//             for (auto* bucket : map_) {
//                 delete bucket;
//             }
//         }

//         private:

//         std::vector<bucket_t*> map_;
//         mutable std::vector<std::mutex> locks_;
//         size_t map_size_;


//     };



// }


// #endif
