cmake_minimum_required(VERSION 4.1.2)

project(stratakv LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(stratakv
    src/simple_hash_table.cc
        stratakv/misc/shared_pointer.h
        stratakv/internal/neon.h
        stratakv/policy/layout.h
        stratakv/policy/growth.h
        stratakv/policy/group.h
        stratakv/internal/group128.h
        stratakv/internal/control_bytes.h
        stratakv/internal/hardware.h
)

target_include_directories(stratakv
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
    PUBLIC 
        ${PROJECT_SOURCE_DIR}/stratakv
)

include(CTest)
enable_testing()

target_compile_options(stratakv PRIVATE -Wall -Wextra -Werror -Wpedantic -Wconversion -Wshadow)


add_subdirectory(tests)
add_subdirectory(bench)