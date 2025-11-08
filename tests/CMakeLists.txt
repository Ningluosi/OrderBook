cmake_minimum_required(VERSION 3.10)
enable_testing()

add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/googletest googletest_build)

file(GLOB TEST_SRC *.cpp)

add_executable(orderbook_tests ${TEST_SRC})

target_include_directories(orderbook_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/third_party/googletest/googletest/include
)

target_link_libraries(orderbook_tests
    PRIVATE
        gtest
        gtest_main
        orderbook_core
        orderbook_engine
        orderbook_dispatch
        orderbook_net
        orderbook_utils
        pthread
)

target_compile_definitions(orderbook_tests PRIVATE LOG_LEVEL=0)

include(GoogleTest)
gtest_discover_tests(orderbook_tests)
