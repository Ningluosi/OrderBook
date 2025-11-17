file(GLOB ENGINE_SRC *.cpp)
add_library(matchengine_engine STATIC ${ENGINE_SRC})
target_include_directories(matchengine_engine PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(matchengine_engine PUBLIC matchengine_core matchengine_dispatch matchengine_utils)
