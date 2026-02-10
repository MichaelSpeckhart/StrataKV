include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.4
)
FetchContent_MakeAvailable(Catch2)

enable_testing()

add_executable(stratakv_tests
  flat_table_test.cc
)

target_link_libraries(stratakv_tests
  PRIVATE
    Catch2::Catch2WithMain
    stratakv
)

include(CTest)
include(Catch)
catch_discover_tests(stratakv_tests)
