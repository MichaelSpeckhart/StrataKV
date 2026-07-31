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
#include <mach/vm_statistics.h>

#ifndef STRATAKV_ARENA_ALLOCATOR_H
#define STRATAKV_ARENA_ALLOCATOR_H


namespace stratakv {


struct alignas(8) ArenaStats
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

    explicit Arena(const size_t num_bytes) : total_size(num_bytes), offset(0)
    {
        offset = 0;
        #ifdef STRATAKV_DEBUG_ALLOCATIONS
        std::cout << "Arena Constructor: Initial Offset: " << offset << std::endl;
        #endif
        const auto page = static_cast<size_t>(sysconf(_SC_PAGESIZE));
        constexpr size_t SUPERPAGE_2MB = 2 * 1024 * 1024;
        const size_t rounded = align_up(num_bytes, SUPERPAGE_2MB);

        // std::cout << "Rounded Alignment: " << rounded << std::endl;

        //int superpage_fd = VM_FLAGS_SUPERPAGE_SIZE_2MB;

        constexpr off_t mmap_offset = 0;
        void* p = MAP_FAILED;

//#if defined(__APPLE__) && defined(VM_FLAGS_SUPERPAGE_SIZE_2MB)
        // 2. Attempt 2 MiB Superpage allocation on macOS
        int superpage_fd = VM_FLAGS_SUPERPAGE_SIZE_2MB;
        p = ::mmap(nullptr, rounded,
                   PROT_READ | PROT_WRITE,
                   MAP_ANON | MAP_PRIVATE,
                   superpage_fd, 0);
//#endif

        // 3. Fallback to standard 16 KiB page mmap if superpages fail or are unsupported
        if (p == MAP_FAILED) {
            p = ::mmap(nullptr, rounded,
                       PROT_READ | PROT_WRITE,
                       MAP_ANON | MAP_PRIVATE,
                       -1, 0);
        }

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

    Arena(const Arena&) = delete;

    Arena& operator=(const Arena&) = delete;

    ~Arena()
    {
        if (base)
        {
            munmap(base, total_size);
        }
    }

    void* allocate(const size_t num_bytes, std::size_t alignment = alignof(std::max_align_t))
    {
        size_t rounded = align_up(num_bytes, static_cast<size_t>(alignment));
        const std::uintptr_t curr = reinterpret_cast<std::uintptr_t>(base) + offset;
        const std::uintptr_t aligned = align_up(curr, static_cast<std::size_t>(alignment));
        const auto new_offset = static_cast<std::size_t>(aligned - reinterpret_cast<std::uintptr_t>(base) + num_bytes);

        
        if (new_offset > total_size)
        {
            return nullptr;
        }

        const auto result = reinterpret_cast<void*>(aligned);

        if (result && num_bytes > 0) {
            volatile auto* p = static_cast<volatile uint8_t*>(result);
            p[0] = 0x00;                    
            if (num_bytes > 4096)           
                p[4096] = 0x00;             
        }

        offset = new_offset;
        return result;
    }

    void deallocate(void* v, size_t n)
    {
        munmap(v, n);
    }


    [[nodiscard]] ArenaStats stats() const
    {
        ArenaStats stats{};

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
    // void destroy()
    // {
    //     offset = 0;
    //     munmap(base, total_size);
    //     base = nullptr;
    //     total_size = 0;
    // }

    private:

    std::byte* region{};
    std::byte* base{};
    size_t total_size{};
    std::size_t offset{};




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
    explicit ArenaAllocator(const ArenaAllocator<U>& other) noexcept : arena_(other.arena_) {}

    template <typename U>
        friend class ArenaAllocator;

    T* allocate(const std::size_t n, const std::size_t alignment = alignof(T)) {
        return static_cast<T*>(arena_->allocate(n * sizeof(T), alignment));
    }

    void deallocate(T* p /*p*/, std::size_t /*n*/ n) noexcept {
        arena_->deallocate(p, n);
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


}

#endif
