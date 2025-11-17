cmake_minimum_required(VERSION 3.16)
project(MatchingEngine CXX)
set(CMAKE_CXX_STANDARD 17)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
)

# -------------------------------------------------------------
# Logging configuration
# -------------------------------------------------------------
set(LOG_LEVEL "3" CACHE STRING "Log verbosity level (0=none, 1=error, 2=warn, 3=info)")
set_property(CACHE LOG_LEVEL PROPERTY STRINGS "0" "1" "2" "3")

add_compile_definitions(LOG_LEVEL=${LOG_LEVEL})
message(STATUS "Build with LOG_LEVEL=${LOG_LEVEL}")


add_subdirectory(src)
add_subdirectory(tests)
