cmake_minimum_required(VERSION 3.26)
project(nimlib)

set(CMAKE_CXX_STANDARD 17)

add_library(nimlib src/main.cpp)

# Include Google Test (downloads automatically if not present)
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/f8d7d77c06936315286eb55f8de22cd23c188571.zip
)
# For Windows: prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Link against gtest or gtest_main as needed
add_executable(unit_tests tests/main.test.cpp)
target_link_libraries(unit_tests GTest::gtest_main)
add_test(NAME unit_tests COMMAND unit_tests)

enable_testing()
include(GoogleTest)
gtest_discover_tests(unit_tests)