cmake_minimum_required(VERSION 3.10)
project(NearProductionOrderBook CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# 子目录模块
add_subdirectory(src)
add_subdirectory(tests)
