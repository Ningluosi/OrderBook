# src/CMakeLists.txt

file(GLOB_RECURSE CORE_SRC core/*.cpp)
add_library(orderbook_core STATIC ${CORE_SRC})
target_include_directories(orderbook_core PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(orderbook_core pthread)

# 主程序（可选）
add_executable(orderbook_main main.cpp)
target_link_libraries(orderbook_main orderbook_core pthread)
