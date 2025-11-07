cmake_minimum_required(VERSION 3.10)
project(OrderBookEngine CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 源文件组织 ---
file(GLOB_RECURSE CORE_SRC src/core/*.cpp)
file(GLOB_RECURSE ENGINE_SRC src/engine/*.cpp)
file(GLOB_RECURSE UTILS_SRC src/utils/*.cpp)
# 如果你有 dispatch 目录（dispatcher、lockfreequeue等）
file(GLOB_RECURSE DISPATCH_SRC src/dispatch/*.cpp)

# --- 静态库 ---
add_library(orderbook_core STATIC
    ${CORE_SRC}
    ${ENGINE_SRC}
    ${UTILS_SRC}
    ${DISPATCH_SRC}
)

# --- 头文件路径 ---
target_include_directories(orderbook_core PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# --- 链接依赖 ---
target_link_libraries(orderbook_core pthread)

# --- 主程序 ---
add_executable(orderbook_main src/main.cpp)
target_link_libraries(orderbook_main orderbook_core pthread)
