file(GLOB CORE_SRC *.cpp)
add_library(core STATIC ${CORE_SRC})
target_include_directories(core PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(core PUBLIC pthread)
