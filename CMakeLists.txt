cmake_minimum_required(VERSION 3.10)
project(NearProductionOrderBook CXX)

# 基本设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

# 包含头文件
include_directories(include)

# 源码路径
file(GLOB_RECURSE CORE_SRC src/core/*.cpp)
set(MAIN_SRC src/main.cpp)

# 核心库（撮合逻辑、对象池等）
add_library(orderbook_core STATIC ${CORE_SRC})
target_link_libraries(orderbook_core pthread)

# 主程序
add_executable(orderbook_main ${MAIN_SRC})
target_link_libraries(orderbook_main orderbook_core pthread)

# 单元测试
if(EXISTS "${CMAKE_SOURCE_DIR}/tests/test_order_book.cpp")
    add_executable(test_order_book tests/test_order_book.cpp)
    target_link_libraries(test_order_book orderbook_core pthread)
endif()
