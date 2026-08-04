#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <iostream>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <stddef.h>

#define NO_FD -1
#define ZERO_OFFSET 0

#define DEADBEEF 0xDEADBEEF


void int_array_fill(int* arr, int val, size_t len)
{
    for (int i = 0; i < len; ++i)
    {
        arr[i] = val;
    }
}


typedef struct linear_allocator
{
    size_t amount_used;
    size_t total_size;
    size_t mapped_size;
    char* base;
} linear_allocator;

static inline size_t align_up(size_t x, size_t a)
{
    // assert((a & (a - 1)) == 0);
    size_t aligned_val =  (x + (a - 1)) & ~(a - 1);

    // printf("Aligned value: %lu\n", aligned_val);

    return aligned_val;
}

linear_allocator* linear_allocator_init(size_t num_bytes)
{
    // printf("Num Bytes: %lu\n", num_bytes);
    size_t page = (size_t)sysconf(_SC_PAGESIZE);
    // printf("Page Size: %lu\n", page);
    num_bytes = align_up(num_bytes, page);

    void* region = mmap(NULL, num_bytes, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, NO_FD, ZERO_OFFSET);

    // printf("Num Bytes: %lu\n", num_bytes);

    if (region == MAP_FAILED)
    {
        perror("mmap");
        return NULL;
    }

    linear_allocator* allocator = (linear_allocator*)region;

    // Alignment Start
    size_t header_size = align_up(sizeof(linear_allocator), _Alignof(max_align_t));
    allocator->amount_used = 0;
    allocator->total_size = num_bytes - header_size;
    // printf("Allocator Total Size: %lu\n", allocator->total_size);
    allocator->mapped_size = num_bytes;
    allocator->base = (char*)region + header_size;
    // Alignment End

    // printf("Header size: %lu\n", header_size);
    return allocator;
}

void* allocate(linear_allocator* allocator, size_t num_bytes, size_t align)
{
    if (!allocator || num_bytes == 0) return NULL;

    uintptr_t current_ptr = (uintptr_t)allocator->base + allocator->amount_used;

    // printf("Current Pointer: %lu\n", current_ptr);

    uintptr_t aligned_ptr = (current_ptr + (align - 1)) & ~(align - 1);

    // printf("Aligned Pointer: %lu\n", aligned_ptr);

    size_t offset_from_base = aligned_ptr - (uintptr_t)allocator->base;
    size_t total_needed = offset_from_base + num_bytes;

    if (total_needed <= allocator->total_size)
    {
        // printf("Total Needed: %lu\n", total_needed);
        allocator->amount_used = total_needed;
        memset((void*)aligned_ptr, 0, allocator->amount_used);
        //(void*)aligned_ptr[allocator->amount_used] = 0xDEADBEEF;
        return (void*)aligned_ptr;
    }
    return NULL;
}

void linear_allocator_destroy(linear_allocator* allocator)
{
    if (!allocator) return;
    munmap(allocator, allocator->mapped_size);
}





int main() {

    int pid = getpid();

    printf("PID: %d\n", pid);

    printf("Size_t size: %lu\n", sizeof(linear_allocator));




    linear_allocator* allocator = linear_allocator_init(1 << 20);

    // printf("Sizeof(linear_allocator): %lu \nAlignOf(linear_allocator): %lu\n", sizeof(linear_allocator), _Alignof(linear_allocator));

    // char* letter = (char*)allocate(allocator, sizeof(char), 16);

    // memset(letter, 'c', 1);

    // printf("Letter: %c\n", *letter);


    // // c
    // int* x = (int*)allocate(allocator, sizeof(int), 16);
    // // *x = 5;



    // printf("X: %d\n", *x);
    // printf("Address of letter: %p\n", (void*)letter);
    // printf("Address of x: %p\n", (void*)x);

    // size_t array_len = 5012;
    // int *arr = (int*)allocate(allocator, array_len * sizeof(int), 16);




    // if (!arr)
    // {
    //     printf("ALLOCATION FAILED\n");
    // }


    // // Common Pitfall: Memset wont work on this array! It works well with 1-byte index arrays though
    // //memset(arr, 1, sizeof(*arr));

    // //int_array_fill(arr, 10, 10);

    // printf("Address of arr: %p\n", (void*)arr);
    //
    std::cout << "Standard Alignment: " << alignof(std::align_val_t ) << std::endl;

    uintptr_t val = reinterpret_cast<uintptr_t>(allocate(allocator, 7, 8)) % 8;

    auto is_aligned = val == 0;
    std::cout << "Val: " << val << std::endl;

    std::cout << "Is Aligned: " << is_aligned << std::endl;

    getchar();






    // munmap(p, sz);
    // munmap(z, sz);

}
