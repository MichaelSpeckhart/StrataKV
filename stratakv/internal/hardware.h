//
// Created by Michael Speckhart on 8/3/26.
//

#ifndef STRATAKV_HARDWARE_H
#define STRATAKV_HARDWARE_H

#include <new>
#include <cstddef>

#if defined(__APPLE__) && defined(__arm64__)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#if defined(__APPLE__) && defined(__arm64__)
    constexpr size_t CompileTimeCacheLine = 128;
#elif defined(__cpp_lib_hardware_interference_size)
constexpr size_t CompileTimeCacheLine = std::hardware_destructive_interference_size;
#else
constexpr size_t CompileTimeCacheLine = 64;
#endif


static constexpr std::size_t cache_line_size() {
#if defined(__APPLE__) && defined(__arm64__)
    // Apple Silicon path: dynamic query to handle 128-byte hardware lines
    size_t line_size = 0;
    size_t size = sizeof(line_size);
    if (sysctlbyname("hw.cachelinesize", &line_size, &size, nullptr, 0) == 0) {
        return line_size;
    }
#endif

    // Non-Apple or fallback path: use modern C++ standard constant if available
#if defined(__cpp_lib_hardware_interference_size)
    return std::hardware_destructive_interference_size;
#else
    return 64; // Absolute safe fallback for older compilers
#endif
}

#endif //STRATAKV_HARDWARE_H
