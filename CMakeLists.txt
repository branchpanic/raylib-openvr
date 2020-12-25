cmake_minimum_required(VERSION 3.16)

if (DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif ()

project(raylib_openvr)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

# Vcpkg seems to have the occasional problem with raylib, so we have included its auto-fetch script. If vcpkg's
# distribution of raylib does not work, uninstall it to fetch the latest release.
# https://github.com/raysan5/raylib/blob/e25e380e80a117f2404d65b37700fb620dc1f990/projects/CMake/CMakeLists.txt
find_package(raylib 3.0 QUIET)

# This part downloads raylib and builds it if it's not installed on your system
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
    include(FetchContent)

    FetchContent_Declare(
            raylib
            URL https://github.com/raysan5/raylib/archive/master.tar.gz
    )

    FetchContent_GetProperties(raylib)
    if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
        set(FETCHCONTENT_QUIET NO)
        FetchContent_Populate(raylib)

        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples

        # build raylib
        add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})

    endif ()
endif ()
link_libraries(raylib)

find_path(OPENVR_INCLUDE_DIR openvr.h)
find_library(OPENVR_LIB openvr_api)
include_directories(${OPENVR_INCLUDE_DIR})
link_libraries(${OPENVR_LIB})

add_executable(main main.cpp rlvr.h rlvr.cpp)
