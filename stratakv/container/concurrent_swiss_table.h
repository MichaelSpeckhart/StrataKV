#pragma once

#ifndef STRATAKV_CONCURRENT_SWISS_TABLE_H
#define STRATAKV_CONCURRENT_SWISS_TABLE_H

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <sys/types.h>
#include <type_traits>
#include <iostream>

using ctrl_t = std::uint8_t;

static constexpr ctrl_t CTRL_EMPTY = 0xFF;
static constexpr ctrl_t CTRL_DELETED = 0XFE;

constexpr double LOAD_FACTOR = 0.9;

#if defined (__MACH__)


template <typename k, typename v>
requires (std::is_trivially_copyable_v<k> && std::is_trivially_copyable_v<v>
&& std::is_trivially_destructible_v<k> && std::is_trivially_destructible_v<v>)
class concurrent_flat_hash_map
{

  private:
  size_t table_size_;

  size_t num_locks = 0;


  public:

  concurrent_flat_hash_map(size_t table_size) : table_size_(table_size)
  {




  }



};


#endif




#endif
