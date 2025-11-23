add_executable(perf_engine_benchmark
    perf_engine_benchmark.cpp
)

target_link_libraries(perf_engine_benchmark
    PRIVATE
        core
        engine
        dispatch
        net
        utils
        pthread
)

target_compile_definitions(perf_engine_benchmark PRIVATE PERF_TEST)


