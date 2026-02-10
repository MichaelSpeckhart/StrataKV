#pragma once

#include <cstddef>


const size_t MAX_KEY_SIZE = 256;
const size_t MAX_VALUE_SIZE = 1024;

const size_t DEFAULT_TABLE_SIZE = 1024;
const float LOAD_FACTOR_THRESHOLD = 0.75f;
const size_t RESIZE_FACTOR = 2;
const size_t INITIAL_BUCKET_COUNT = 16;
const size_t BUCKET_SIZE_LIMIT = 8;
const size_t MAX_TABLE_SIZE = 1 << 20; // 1 million entries

const size_t CACHE_LINE_SIZE = 64; // in bytes
const size_t MEMORY_PAGE_SIZE = 4096; // in bytes
const size_t IO_BUFFER_SIZE = 8192; // in bytes
const size_t MAX_IO_RETRIES = 3;
const size_t IO_RETRY_DELAY_MS = 100; // in milliseconds
const size_t DEFAULT_THREAD_POOL_SIZE = 4;
const size_t MAX_THREAD_POOL_SIZE = 32;
const size_t LOG_FLUSH_INTERVAL_MS = 500; // in milliseconds
