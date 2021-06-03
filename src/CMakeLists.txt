
# find_package(OpenGL REQUIRED)

add_library(avionics
STATIC
    display.c
    renderer.c
    module.c
    dref.c
    cmd.c
    glad.c
    gl.c
    xplane.c
)
target_compile_features(avionics PUBLIC c_std_11)

target_include_directories(avionics
    PUBLIC $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# link_directories(avionics PUBLIC "${LIBAV_DEPS_DIR}/lib")
target_include_directories(avionics
    PUBLIC
        "${CMAKE_SOURCE_DIR}/include"
        "${LIBAV_DEPS_DIR}/include"
        "${LIBAV_DEPS_DIR}/include/freetype2"
    PRIVATE ${OPENGL_INCLUDE_DIRS}
)

target_link_directories(avionics PUBLIC "${LIBAV_DEPS_DIR}/lib")

set_target_properties(avionics
PROPERTIES
   C_VISIBILITY_PRESET hidden
   VISIBILITY_INLINES_HIDDEN ON
   POSITION_INDEPENDENT_CODE ON
)


target_include_directories(avionics PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/sdk/CHeaders/XPLM")
target_link_libraries(avionics PUBLIC ccore::ccore cairo pixman-1 freetype z pthread)
target_compile_options(avionics PUBLIC -Wall -Wextra -pedantic -Werror)

message(STATUS "headers: ${CMAKE_CURRENT_SOURCE_DIR}/sdk/CHeaders/XPLM")

if(WIN32)
	add_definitions(-DAPL=0 -DIBM=1 -DLIN=0)
elseif(APPLE)
	add_definitions(-DAPL=1 -DIBM=0 -DLIN=0)
else()
	add_definitions(-DAPL=0 -DIBM=0 -DLIN=1)
endif()
add_definitions(-DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1)

set_target_properties(avionics
PROPERTIES
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
    LINK_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fno-stack-protector"
)

if(APPLE)
    find_library(XPLM_LIBRARY XPLM ${CMAKE_CURRENT_SOURCE_DIR}/sdk/Libraries/Mac/)
    find_library(XPWIDGETS_LIBRARY XPWidgets ${CMAKE_CURRENT_SOURCE_DIR}/sdk/Libraries/Mac/)
    
    message(STATUS "xplm: ${XPLM_LIBRARY}")
    # target_link_libraries(avionics PUBLIC ${XPLM_LIBRARY})
    # target_link_libraries(avionics PUBLIC ${XPWIDGETS_LIBRARY})
elseif(WIN32)
    set(CMAKE_LIBRARY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/sdk/Libraries/Win/)
    find_library(XPLM_LIBRARY
        NAMES XPLM_64
        PATHS ${CMAKE_CURRENT_SOURCE_DIR}/sdk/Libraries/Win/
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH
    )
    find_library(XPWIDGETS_LIBRARY
        XPWidgets_64
        ${CMAKE_CURRENT_SOURCE_DIR}/sdk/Libraries/Win/
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH
    )
    # target_link_libraries(xnavmap PUBLIC ${XPLM_LIBRARY})
    # target_link_libraries(xnavmap PUBLIC ${XPWIDGETS_LIBRARY})
endif()
target_link_libraries(avionics PUBLIC ccore::ccore cairo pixman-1 freetype z pthread)
