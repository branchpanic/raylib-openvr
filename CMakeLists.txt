cmake_minimum_required(VERSION 3.16)

project(
	raylib-openvr-examples
	VERSION
        0.0.0
	DESCRIPTION
        "Examples for OpenVR raylib fork"
)

# Each example is built from one C file under examples/ with the same basename, i.e. "hello_vr" <- "hello_vr.c"
set(
	EXAMPLES
		hello_vr
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# vcpkg install openvr
find_path(OPENVR_INCLUDE_DIR openvr_capi.h)
find_library(OPENVR_LIB openvr_api)
include_directories(${OPENVR_INCLUDE_DIR})
link_libraries(${OPENVR_LIB})

# use our local raylib fork
# https://github.com/raysan5/raylib/blob/551597d5793532b19ed71808803a0d6a6e1178b0/projects/CMake/CMakeLists.txt#L25
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory("raylib")
link_libraries(raylib)

foreach (example ${EXAMPLES})
	add_executable(${example} examples/${example}.c)
endforeach()
