add_executable(perf_tps_engine
    perf_tps_engine.cpp
)

target_link_libraries(perf_tps_engine
    PRIVATE
        core
        engine
        dispatch
        net
        utils
        pthread
)

target_compile_definitions(perf_tps_engine PRIVATE PERF_TEST)


