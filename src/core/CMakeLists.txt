file(GLOB CORE_SRC *.cpp)
add_library(matchengine_core STATIC ${CORE_SRC})
target_include_directories(matchengine_core PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(matchengine_core PUBLIC pthread)
