# Apple Silicon Notes

## Summary of Contents
1. Instruction Set Architecture (ISA) Optimizations Overview
    * ARM AArch64 ISA
    * Syntax & Registers
    * Memory Model, Barriers, and Synchronization
    * ISA Characterization
1. ISA Optimization: Advanced SIMD and FP Unit
    * Data Types for SIMD and FP Data Types
    * Arm Advanced SIMD and FP Registers
    * Arm Advanced Mnemonics
    * Compiler Intrinsics
    * Recommendations
    * Examples
1. Core Microarchitecture Optimization
    * Apple Silicon CPU and Chip Overview
    * Microarchitecture Characteristics
    * Pipeline Fundamentals
    * Instruction Delivery Optimization
        * Instruction Caches, Branching
    * Processing Memory Optimizations
        * L1, Shared L2, Memory Caches, Pointer Chasing
1. Asymmetric Multiprocessor (AMP) Optimization
    * Priotizing and Partitioning 
    * Avoid Spin Wait
    * APIs for Synchronization
1. Xcode Performance Monitoring and Analysis


# Optimization Process
* Create a goal, such as "n frames per second" to achieve 
* Use mach_absolute_time() library to measure time from the `#include <mach/mach_time.h>` header file
* Focus on 2 facets
    * __Instruction Set Architecture (ISA):__ Contains commands software uses to do tasks such as instruction definitions, register serts, virtual memory organization, memory model, etc.
        * __NB__ Choose Instructions and instruction sequences __(Research This)__
    * __Microarchitecture:__ Underlying hardware mechanism that implements ISA. Fetches instructions from memory and executes operations, updating registers often in parallel. 
        * __NB__ Choose instruction patterns, memory access patterns that optimally use available microarchitectural resources such as cache-line capacity (128 byte), instruction type bandwidth, and instruction latencies __(Research This)__
* For Optimizations, apply in order
    * __Compiler Optimizations__
        * Use release configurations and compiler optimizations such as `-O3` or `-Ofast`, however bloat size of code and increase cache misses
    * __Frameworks and Libraries__
    * __Profile Code__
    * __Data Structure Design__
        * Decorate Data (__Research This__), use `const`, `restrict`, `pragma unroll`, and `pragma vectorize`
    * __Use intrinsics or assembly instructions__
    * __Algorithms For Better Caching__
    * __Optimiza for microarchitecture, tuning for maxmimum throughput__
* Instruction Set Architecture: Assembly is the textual definition for ISA. Apple Silicon uses AArch64 v8 and v9


# ISA Optimization: ASIMD and FP Unit
Operations with larger data types reduce the number of parallel operations performed in inverse proportion to
the size of each data element
![Figure Shows Parallel SIMD Instructions for 128-bit data with different type sizes](/assets/Standard_SIMD_Addition.png)
