#pragma once

#include <atomic>
#ifndef STRATAKV_MEMORY_ALLOCATOR_H
#define STRATAKV_MEMORY_ALLOCATOR_H

#include <mach/thread_policy.h>
#include <mach/thread_act.h>

#include <sys/mman.h>

#if defined (TARGET_OS_WIN32)

#endif
#if defined (__MACH__)

#endif
inline void allocate_chunk_internal(size_t chunk_size) {
    mmap(NULL, chunk_size, PROT_EXEC, 1, 1, NULL);


}




#endif
