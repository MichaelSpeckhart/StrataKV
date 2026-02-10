#pragma once

#ifndef STRATAKV_SEQUENTIAL_TABLE_H
#define STRATAKV_SEQUENTIAL_TABLE_H

#include "map.h"
#include "table_bucket.h"
#include <vector>


namespace stratakv {

template <typename key_type, typename value_type>
class sequential_table : public map<key_type, value_type, sequential_table_bucket<key_type, value_type>> {
public:

    sequential_table() {
        // Constructor implementation
    }

    ~sequential_table() override {
        // Destructor implementation
    }

    void insert(const key_type& key, const value_type& value) override {
        // Insert implementation
    }

    bool erase(const key_type& key) override {
        // Erase implementation
        return false;
    }

    std::optional<value_type> get(const key_type& key) const override {
        // Get implementation
        return std::nullopt;
    }

    size_t map_size() const override {
        // Map size implementation
        return 0;
    }

private:
    size_t hash_function(const key_type& key) const override {
        // Hash function implementation
        return 0;
    }

    void resize_table() override {
        // Resize table implementation
    }


    size_t current_size_;

};

}




#endif //STRATAKV_SEQUENTIAL_TABLE_H