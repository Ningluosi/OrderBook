add_subdirectory(core)
add_subdirectory(dispatch)
add_subdirectory(engine)
add_subdirectory(net)
add_subdirectory(utils)

add_executable(orderbook_main main.cpp)
target_link_libraries(orderbook_main
    orderbook_core
    orderbook_engine
    orderbook_dispatch
    orderbook_engine
    orderbook_net
    orderbook_utils
    pthread
)
