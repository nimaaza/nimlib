cmake_minimum_required(VERSION 3.28.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(nimlib VERSION 0.0.1)
configure_file(version.h.in version.h)

find_package(OpenSSL)
message("Using OpenSSL version: ${OPENSSL_VERSION}")

add_executable(test_run src/test_run.cpp src/tcp_socket.cpp src/polling_server.cpp src/connection.cpp src/protocol.cpp)
target_include_directories(test_run PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(test_run OpenSSL::SSL OpenSSL::Crypto)

# TDD ---------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/f8d7d77c06936315286eb55f8de22cd23c188571.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(unit_tests tests/connection.test.cpp src/connection.cpp src/protocol.cpp src/tcp_socket.cpp)
target_link_libraries(unit_tests GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(unit_tests)
# -------------------------------------------------------------------
