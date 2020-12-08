add_executable(demo app.c main.c gl.c glad/glad.c)
target_link_libraries(demo PRIVATE m mfd glfw ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS} ${OPENGL_LIBRARIES} ccore)

add_custom_target(run
    COMMAND demo
    DEPENDS demo
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_custom_target(dbg
    COMMAND lldb $<TARGET_FILE:demo>
    DEPENDS demo
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
