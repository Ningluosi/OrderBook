file(GLOB NET_SRC *.cpp)
add_library(net STATIC ${NET_SRC})
target_include_directories(net PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(net PUBLIC utils)
