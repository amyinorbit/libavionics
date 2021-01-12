
find_package(OpenGL REQUIRED)

add_library(avionics STATIC display.c module)
target_compile_features(avionics PUBLIC c_std_11)
target_include_directories(avionics
    PUBLIC "${CMAKE_SOURCE_DIR}/include"
    PRIVATE ${OPENGL_INCLUDE_DIRS}
)


target_include_directories(avionics
    PUBLIC $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

link_directories(avionics PUBLIC "${LIBAV_DEPS_DIR}/lib")
target_include_directories(avionics PUBLIC
        "${LIBAV_DEPS_DIR}/include"
        "${LIBAV_DEPS_DIR}/include/freetype2"
)

set_target_properties(avionics
PROPERTIES
   C_VISIBILITY_PRESET hidden
   VISIBILITY_INLINES_HIDDEN ON
   POSITION_INDEPENDENT_CODE ON
)

target_link_libraries(avionics PUBLIC ${OPENGL_LIBRARIES} cairo pixman-1 freetype pthread)
