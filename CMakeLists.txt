cmake_minimum_required(VERSION 4.1.2)

project(stratakv LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(stratakv
    src/simple_hash_table.cc
    stratakv/shared_pointer.h
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