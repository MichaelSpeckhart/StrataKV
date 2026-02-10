#pragma once
#ifndef STRATAKV_NUMA_TABLE_H
#define STRATAKV_NUMA_TABLE_H


#include <numa_table_entry.h>

#include <stdint.h>

using ctrl_t = std::uint8_t;

static constexpr ctrl_t CTRL_EMPTY = 0xFF;
static constexpr ctrl_t CTRL_DELETED = 0XFE;

enum class Ctrl : ctrl_t {
    kEmpty = CTRL_EMPTY,
    kDeleted = CTRL_DELETED,
};


typedef struct {
    Ctrl ctrl;
} numaControls;




#endif
