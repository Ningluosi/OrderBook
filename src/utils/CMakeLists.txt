file(GLOB UTILS_SRC *.cpp)
add_library(orderbook_utils STATIC ${UTILS_SRC})
target_include_directories(orderbook_utils PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(orderbook_utils PUBLIC pthread)
