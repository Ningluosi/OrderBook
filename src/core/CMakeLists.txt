file(GLOB CORE_SRC *.cpp)
add_library(orderbook_core STATIC ${CORE_SRC})
target_include_directories(orderbook_core PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(orderbook_core PUBLIC pthread)
