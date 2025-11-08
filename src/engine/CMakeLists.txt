file(GLOB ENGINE_SRC *.cpp)
add_library(orderbook_engine STATIC ${ENGINE_SRC})
target_include_directories(orderbook_engine PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(orderbook_engine PUBLIC orderbook_core orderbook_dispatch orderbook_utils)
