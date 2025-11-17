file(GLOB NET_SRC *.cpp)
add_library(matchengine_net STATIC ${NET_SRC})
target_include_directories(matchengine_net PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(matchengine_net PUBLIC matchengine_utils)
