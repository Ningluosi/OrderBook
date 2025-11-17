file(GLOB DISPATCH_SRC *.cpp)
add_library(matchengine_dispatch STATIC ${DISPATCH_SRC})
target_include_directories(matchengine_dispatch PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(matchengine_dispatch PUBLIC matchengine_utils)
