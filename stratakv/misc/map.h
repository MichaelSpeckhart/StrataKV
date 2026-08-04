#pragma once

#ifndef STRATAKV_MAP_H
#define STRATAKV_MAP_H

#include <cstdint>
#include <optional>


namespace stratakv {

template <typename key_type, typename value_type>
class Map {
public:

    virtual ~Map() {}

    virtual bool insert(const key_type& key, const value_type& value) = 0;
    virtual bool erase(const key_type& key) = 0;
    virtual std::optional<value_type> get(const key_type& key) const = 0;
    virtual size_t map_size() const = 0;


private:
    
    //virtual size_t hash_function(const key_type& key) const = 0;
    //virtual void resize_table() = 0;

};


}


#endif //STRATAKV_MAP_H