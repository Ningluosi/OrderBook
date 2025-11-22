file(GLOB DISPATCH_SRC *.cpp)
add_library(dispatch STATIC ${DISPATCH_SRC})
target_include_directories(dispatch PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(dispatch PUBLIC utils)
