cmake_minimum_required(VERSION 3.16)
project(MatchingEngine CXX)
set(CMAKE_CXX_STANDARD 17)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
)

option(UNIT_TEST "Enable unit test mode" OFF)
option(PERF_TEST "Enable performance test mode" OFF)

if (UNIT_TEST)
    set(LOG_LEVEL 0)
    message(STATUS "UNIT_TEST mode → LOG_LEVEL=0 (silent)")
elseif (PERF_TEST)
    set(LOG_LEVEL 0)
    message(STATUS "PERF_TEST mode → LOG_LEVEL=0 (silent)")
else()
    set(LOG_LEVEL 3 CACHE STRING "Log verbosity level (0..4)" FORCE)
    message(STATUS "NORMAL mode → LOG_LEVEL=${LOG_LEVEL}")
endif()


add_compile_definitions(LOG_LEVEL=${LOG_LEVEL})
if (UNIT_TEST)
    add_compile_definitions(UNIT_TEST)
endif()
if (PERF_TEST)
    add_compile_definitions(PERF_TEST)
endif()

add_subdirectory(src)
add_subdirectory(tests/unit)
add_subdirectory(tests/perf)
