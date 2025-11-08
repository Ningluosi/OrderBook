file(GLOB DISPATCH_SRC *.cpp)
add_library(orderbook_dispatch STATIC ${DISPATCH_SRC})
target_include_directories(orderbook_dispatch PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(orderbook_dispatch PUBLIC orderbook_utils)
