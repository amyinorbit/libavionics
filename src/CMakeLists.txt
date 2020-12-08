
find_package(OpenGL REQUIRED)

add_library(avionics STATIC display.c)
target_compile_features(avionics PUBLIC c_std_11)
target_include_directories(avionics
    PUBLIC "${CMAKE_SRC_DIR}/include"
    PRIVATE ${OPENGL_INCLUDE_DIRS}
)

target_link_libraries(avionics PUBLIC ${OPENGL_LIBRARIES} cairo pixman-1 freetype pthread)
