file(GLOB NET_SRC *.cpp)
add_library(orderbook_net STATIC ${NET_SRC})
target_include_directories(orderbook_net PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(orderbook_net PUBLIC orderbook_utils)
