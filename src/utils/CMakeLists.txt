file(GLOB UTILS_SRC *.cpp)
add_library(matchengine_utils STATIC ${UTILS_SRC})
target_include_directories(matchengine_utils PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(matchengine_utils PUBLIC pthread)
