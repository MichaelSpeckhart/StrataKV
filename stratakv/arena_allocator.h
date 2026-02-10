#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <forward_list>
#include <iterator>
#include <memory_resource>
#include <new>

#include <memory>
#include <numeric>
#include <stdexcept>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>
#include <vector>

#ifndef STRATAKV_ARENA_ALLOCATOR_H
#define STRATAKV_ARENA_ALLOCATOR_H

#define STRATAKV_DEBUG_ALLOCATIONS

namespace stratakv {


struct alignas(128) ArenaStats
{
    size_t bytes_used; // 8 bytes
    size_t bytes_free; // 8 bytes
    size_t bytes_unusable; // 8 bytes
    size_t total_size; // 8 bytes
    size_t blocks; // 8 bytes
    // Total  40 bytes
};

class Arena
{

    public:

    constexpr Arena() = default;

    explicit Arena(size_t num_bytes) : total_size(num_bytes), offset(0)
    {
        offset = 0;
        #ifdef STRATAKV_DEBUG_ALLOCATIONS
        std::cout << "Arena Constructor: Initial Offset: " << offset << std::endl;
        #endif
        size_t page = static_cast<size_t>(sysconf(_SC_PAGESIZE));
        size_t rounded = align_up(num_bytes, page);

        // std::cout << "Rounded Alignment: " << rounded << std::endl;

        void* p = ::mmap(nullptr, rounded, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);

        if (p == MAP_FAILED)
        {
            #ifdef STRATAKV_DEBUG_ALLOCATIONS
            std::cerr << "Arena mmap failed for " << rounded << " bytes. errno=" << errno << std::endl;
            #endif
            throw std::runtime_error("mmap failed (errno=" + std::to_string(errno) + ")");
        }

        base = static_cast<std::byte*>(p);
        total_size = rounded;
    }

    ~Arena()
    {
        if (base)
        {
            munmap(base, total_size);
        }
    }

    void* allocate(size_t num_bytes, std::size_t alignment = alignof(std::max_align_t))
    {
        size_t rounded = align_up(num_bytes, static_cast<size_t>(alignment));
        std::uintptr_t curr = reinterpret_cast<std::uintptr_t>(base) + offset;
        std::uintptr_t aligned = align_up(curr, static_cast<std::size_t>(alignment));
        const std::uintptr_t new_offset = static_cast<std::size_t>(aligned - reinterpret_cast<std::uintptr_t>(base) + num_bytes);

        
        if (new_offset > total_size)
        {
            return nullptr;
        }

        void* result = reinterpret_cast<void*>(aligned);

        if (result && num_bytes > 0) {
            volatile uint8_t* p = static_cast<volatile uint8_t*>(result);
            p[0] = 0x00;                    
            if (num_bytes > 4096)           
                p[4096] = 0x00;             
        }

        offset = new_offset;
        return result;
    }

    void deallocate(void* v, size_t n)
    {
        // empty
    }


    ArenaStats stats() const
    {
        ArenaStats stats;

        stats.blocks = 0;
        stats.bytes_free = 0;
        stats.bytes_used = 0;
        stats.total_size = 0;
        stats.bytes_unusable = 0;

        return stats;
    }

    // Reset the arena to reuse memory
    void reset()
    {
        offset = 0;
    }

    // Destroy all objects but keep the allocated memory
    void destroy()
    {
        offset = 0;
        munmap(base, total_size);
        base = nullptr;
        total_size = 0;
    }

    private:

    std::byte* region;
    std::byte* base;
    size_t total_size;
    std::size_t offset;


    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;

    static std::size_t align_up(std::size_t x, std::size_t a)
    {
        return (x + (a - 1)) & ~(a - 1);
    }

};


template <typename T>
class ArenaAllocator
{
    public:

    using value_type = T;

    explicit ArenaAllocator(Arena* arena) noexcept : arena_(arena) {}

    template <typename U>
        ArenaAllocator(const ArenaAllocator<U>& other) noexcept : arena_(other.arena_) {}

    template <typename U>
        friend class ArenaAllocator;

    T* allocate(std::size_t n, std::size_t alignment = alignof(T)) {
        return static_cast<T*>(arena_->allocate(n * sizeof(T), alignment));
    }

    void deallocate(T* p /*p*/, std::size_t /*n*/ n) noexcept {
        // No-op; arena manages memory lifetime
        arena_->destroy();
    }

    bool operator==(const ArenaAllocator& other) const noexcept {
        return arena_ == other.arena_;
    }

    bool operator!=(const ArenaAllocator& other) const noexcept {
        return !(*this == other);
    }

    private:

    Arena* arena_;
};

// static_assert(sizeof(ArenaAllocator) <= (size_t)sysconf(_SC_PAGESIZE), "Arena is too large");
//
template <typename value_type, typename allocator_type>
class StrataAllocator
{

    public:

    explicit StrataAllocator(allocator_type* allocator) noexcept : allocator_(allocator) {}

    template <typename U, typename V>
        StrataAllocator(const StrataAllocator<U, V>& other) noexcept : allocator_(other.allocator_) {}

    template <typename U, typename V>
        friend class StrataAllocator;

    value_type* allocate(std::size_t n)
    {
        return static_cast<value_type*>(allocator_->allocate(n*sizeof(value_type), alignof(value_type)));
    }

    void deallocate(value_type* v, size_t n)
    {
        allocator_->deallocate(v, n);
    }


    private:

    allocator_type* allocator_;
};

static_assert(true, "");

}

#endif
