file(GLOB UTILS_SRC *.cpp)
add_library(utils STATIC ${UTILS_SRC})
target_include_directories(utils PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(utils PUBLIC pthread)
