
find_package(OpenGL REQUIRED)

add_library(mfd STATIC display.c)
target_compile_features(mfd PUBLIC c_std_11)
target_include_directories(mfd PUBLIC "${CMAKE_SRC_DIR}/include" PRIVATE ${OPENGL_INCLUDE_DIRS})
target_link_libraries(mfd PUBLIC ${OPENGL_LIBRARIES} cairo pixman-1 freetype pthread)
