cmake_minimum_required(VERSION 3.10)
enable_testing()

add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/googletest googletest_build)

file(GLOB TEST_SRC *.cpp)

add_executable(matchingengine_test ${TEST_SRC})

target_include_directories(matchingengine_test PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/third_party/googletest/googletest/include
)

target_link_libraries(matchingengine_test
    PRIVATE
        gtest
        gtest_main
        core
        engine
        dispatch
        net
        utils
        pthread
)

include(GoogleTest)
gtest_discover_tests(matchingengine_test)
