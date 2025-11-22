file(GLOB ENGINE_SRC *.cpp)
add_library(engine STATIC ${ENGINE_SRC})
target_include_directories(engine PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(engine PUBLIC core dispatch utils)
