cmake_minimum_required(VERSION 3.10)

add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/googletest googletest_build)
enable_testing()

file(GLOB TEST_SRC *.cpp)
set(CORE_SRC ${PROJECT_SOURCE_DIR}/src/core/order_book.cpp)

add_executable(orderbook_tests ${TEST_SRC} ${CORE_SRC})

target_include_directories(orderbook_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(orderbook_tests PRIVATE gtest gtest_main)

target_compile_definitions(orderbook_tests PRIVATE LOG_LEVEL=0)

include(GoogleTest)
gtest_discover_tests(orderbook_tests)
