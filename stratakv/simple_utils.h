#pragma once

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <new>
#ifndef STRATAKV_SIMPLE_UTILS_H
#define STRATAKV_SIMPLE_UTILS_H

#include <cstdint>
#include <iostream>

void* strata_malloc(std::size_t bytes)
{
    void* p = std::malloc(bytes);

    if (!p) {
        std::fprintf(stderr, "strata_malloc: allocation failed (%zu bytes)\n", bytes);
        throw std::bad_alloc();
    }

    return p;
}

size_t strata_hash(size_t &key, size_t size) {
    size_t hash = key & size;
    auto h2 = (hash >> 17) & 0x7F;

    std::cout << h2 << std::endl;

    return hash;
}

#endif
