cmake_minimum_required(VERSION 3.12)
project(mfd VERSION 0.0.1 LANGUAGES C)

if(APPLE)
    message("building with macOS support")
    set(PLATFORM_NAME "macos")
    link_directories(/usr/local/lib)
elseif(WIN32)
    message("building with Windows support")
    set(PLATFORM_NAME "windows")
else()
    message("building with Linux support")
    set(PLATFORM_NAME "linux")
endif()


link_directories("${CMAKE_SOURCE_DIR}/lib/build-${PLATFORM_NAME}/lib")
include_directories("${CMAKE_SOURCE_DIR}/lib/build-${PLATFORM_NAME}/include")
include_directories("${CMAKE_SOURCE_DIR}/lib/build-${PLATFORM_NAME}/include/freetype2")
option(LIBMFD_BUILD_DEMO "Build a glfw-based demo" OFF)

add_definitions(
    "-DLIBMFD_VERSION=\"${PROJECT_VERSION}\""
    "-DLIBMFD_VERSION_MAJOR=${PROJECT_MAJOR_VERSION}"
    "-DLIBMFD_VERSION_MINOR=${PROJECT_MINOR_VERSION}"
    "-DLIBMFD_VERSION_PATCH=${PROJECT_PATCH_VERSION}"
)
#
# find_package(PkgConfig REQUIRED)
# set(ENV{PKG_CONFIG_PATH} "${CMAKE_SOURCE_DIR}$ENV{PKG_CONFIG_PATH}")


find_package(Threads REQUIRED)
find_package(ccore 1.6 REQUIRED)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include the SDK. Adjust this path to wherever you have the SDK repository
# add_subdirectory(xplane_sdk)
include_directories(include)
include_directories(src)

add_subdirectory(src)
if(LIBMFD_BUILD_DEMO)
    add_subdirectory(demo)
endif()
