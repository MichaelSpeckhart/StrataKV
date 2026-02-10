#pragma once

#include "numa_table_entry.h"
#include <cstdint>
#include <vector>

#include <arm_neon.h>
#ifndef STRATAKV_NUMA_SHARD_H
#define STRATAKV_NUMA_SHARD_H

enum class NumaShardType { PERFORMANCE = 1, EFFICIENCY = 2};

typedef struct {
    uint32_t shard_id;
    NumaShardType shard_type;
    uint32_t affinity_tag;
} NumaShardIdentifier;

class NumaShard {
    private:
    alignas(4096) std::vector<numaTableBucket> buckets_;
    NumaShardIdentifier identifier_;
};


#endif
