add_subdirectory(core)
add_subdirectory(dispatch)
add_subdirectory(engine)
add_subdirectory(net)
add_subdirectory(utils)

add_executable(matchengine_main main.cpp)
target_link_libraries(matchengine_main
    core
    engine
    dispatch
    engine
    net
    utils
    pthread
)
