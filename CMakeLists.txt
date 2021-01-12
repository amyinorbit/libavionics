cmake_minimum_required(VERSION 3.12)
project(libavionics VERSION 0.0.1 LANGUAGES C)
cmake_policy(SET CMP0077 NEW)

option(LIBAV_DEPS_DIR "Library directory containing Cairo and Freetype" "")
option(LIBMFD_BUILD_DEMO "Build a glfw-based demo" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "dependency directory: ${LIBAV_DEPS_DIR}")

add_definitions(
    "-DLIBAV_VERSION=\"${PROJECT_VERSION}\""
    "-DLIBAV_VERSION_MAJOR=${PROJECT_MAJOR_VERSION}"
    "-DLIBAV_VERSION_MINOR=${PROJECT_MINOR_VERSION}"
    "-DLIBAV_VERSION_PATCH=${PROJECT_PATCH_VERSION}"
)

find_package(Threads REQUIRED)
# find_package(ccore 1.6 REQUIRED)

include_directories(include)
include_directories(src)

add_subdirectory(src)
if(LIBMFD_BUILD_DEMO)
    add_subdirectory(demo)
endif()
