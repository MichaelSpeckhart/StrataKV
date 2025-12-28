cmake_minimum_required(VERSION 4.1.2)

project(stratakv LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(stratakv
    src/simple_hash_table.cc
)

target_include_directories(stratakv
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

include(CTest)
enable_testing()


add_subdirectory(tests)