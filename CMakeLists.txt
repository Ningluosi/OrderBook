cmake_minimum_required(VERSION 3.16)
project(OrderBookEngine CXX)
set(CMAKE_CXX_STANDARD 17)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
)

# -------------------------------------------------------------
# Logging configuration
# -------------------------------------------------------------
option(LOG_LEVEL "Log verbosity level (0=none, 4=debug)" 3)
add_compile_definitions(LOG_LEVEL=${LOG_LEVEL})
message(STATUS "Build with LOG_LEVEL=${LOG_LEVEL}")

add_subdirectory(src)
add_subdirectory(tests)
