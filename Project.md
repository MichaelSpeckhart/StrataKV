# Project Milestones
Milestone 1: Simple Table with One Lock

## Concurrent Hash Table
The starting hash table architecture will be a standard vector, where each index of the vector is a 'bucket'. 
The 'bucket' consists of an std::unordered_map, which is a data structure that holds a key-value pair. This creates fast
insert and lookup times for the table. One would take a key, hash it, find the index for the bucket, and iterate through the bucket. 

## Flat Hash Table (Swiss Table Design)
The bucket design, via std::unordered_map, is a 'deep' design, meaning structurally, in C++ syntax, it is a std::vector of linked-lists, commonly implemented with std::list. 

The flat table design, implemented in the absl library, has a byte array called `controls` where each control is a identifier to whether the corresponding index in `keys` is empty, deleted, or full. This means the hot path is the probing of the `controls` array 

### Locking Strategies
There are a plethora of ways to ensure correctness when dealing with concurrent environments. Locking, where we take a std::mutex, lock it, perform some operation, then unlock it, is the standard pattern.
