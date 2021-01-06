cmake_minimum_required(VERSION 3.16)

project(
	raylib-openvr
    LANGUAGES
        C
	VERSION
        0.0.0
	DESCRIPTION
        "OpenVR companion library for raylib"
)

set(CMAKE_C_STANDARD 11)

# https://github.com/raysan5/raylib/blob/551597d5793532b19ed71808803a0d6a6e1178b0/projects/CMake/CMakeLists.txt#L25
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory("third_party/raylib")
link_libraries(raylib)

if(WIN32)
    # https://github.com/lebek/openvr-input-recorder/blob/15b906bb84efe9df3c13cbb805270ca34775b464/CMakeLists.txt#L42
    find_library(OPENVR_LIBRARIES
        NAMES
            openvr_api
        PATHS
            ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openvr/bin
            ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openvr/lib
        PATH_SUFFIXES
            win64
        NO_DEFAULT_PATH
        NO_CMAKE_FIND_ROOT_PATH
    )
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/openvr/headers)
    link_libraries(${OPENVR_LIBRARIES})
else()
    message(FATAL_ERROR "Non-win32 OpenVR dependency not yet implemented")
endif()

add_library(raylib-openvr rayvr.h rayvr.c)

if(WIN32)
    # Copy openvr_api.dll from OpenVR distribution to build directory
    # Also adapted from openvr-input-recorder
    add_custom_command(
        TARGET
            raylib-openvr
        POST_BUILD 
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/third_party/openvr/bin/win64/openvr_api.dll"              
            $<TARGET_FILE_DIR:raylib-openvr>
    )
endif()
