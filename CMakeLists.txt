cmake_minimum_required(VERSION 3.27.8)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(nimlib VERSION 0.0.1)
configure_file(version.h.in version.h)

find_package(Botan 3.3.0 REQUIRED)
message("Using Botan version: ${BOTAN_VERSION}")
find_package(OpenSSL)
message("Using OpenSSL version: ${OPENSSL_VERSION}")

add_executable(test_run
        test_run.cpp
        src/common/types.h
        src/common/decorators.h
        src/tcp_socket.cpp
        src/polling_server.cpp
        src/connection_pool.cpp
        src/connection.cpp
        src/http.cpp
        src/metrics/timer.cpp
        src/logger/logger.cpp
        src/logger/agent.cpp
        src/logger/factory.cpp
        src/tls_layer.cpp
        src/botan/credentials.cpp
        src/botan/callbacks.cpp
        src/botan/tls_server.cpp)
target_include_directories(test_run PUBLIC "${PROJECT_BINARY_DIR}")
target_compile_definitions(test_run PRIVATE NO_TESTING)
target_link_libraries(test_run Botan::Botan)
#target_link_libraries(test_run OpenSSL::SSL OpenSSL::Crypto)

# TDD ---------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/f8d7d77c06936315286eb55f8de22cd23c188571.zip)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(unit_tests
        tests/polling_server.test.cpp
        tests/state_manager.test.cpp
        tests/metrics/timer.test.cpp
        tests/metrics/aggregations.test.cpp
        tests/metrics/metric_store.test.cpp
        tests/metrics/metric.test.cpp
        tests/metrics/measurements.test.cpp
        src/tcp_socket.cpp
        src/polling_server.cpp
        src/connection.cpp
        src/connection_pool.cpp
        src/http.cpp
        src/metrics/timer.cpp
        src/logger/logger.cpp
        src/logger/agent.cpp
        src/logger/factory.cpp
        src/tls_layer.cpp
        src/botan/credentials.cpp
        src/botan/callbacks.cpp
        src/botan/tls_server.cpp)
target_include_directories(unit_tests PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(unit_tests GTest::gtest_main Botan::Botan)

add_executable(unit_tests_connection
        tests/connection.test.cpp
        tests/support/tcp_socket.mock.cpp
        src/connection.cpp)
target_include_directories(unit_tests_connection PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(unit_tests_connection GTest::gtest_main Botan::Botan)

add_executable(unit_tests_polling_server
        tests/polling_server.test.cpp
        src/polling_server.cpp
        src/connection_pool.cpp
        src/tcp_socket.cpp
        src/connection.cpp
        src/http.cpp) # this should be removed from the dependecies
target_include_directories(unit_tests_polling_server PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(unit_tests_polling_server GTest::gtest_main Botan::Botan)

include(GoogleTest)
gtest_discover_tests(unit_tests unit_tests_connection unit_tests_polling_server)
# -------------------------------------------------------------------
