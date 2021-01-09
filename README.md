# raylib-openvr

Write VR games in C! This project adds OpenVR support to raylib using the OpenVR C API. This project contains examples,
documentation, and issue tracking while [dhsavell/raylib](https://github.com/dhsavell/raylib/tree/openvr) (submoduled
as raylib/) contains the raylib fork.

Raylib-openvr is still in a prototype state. It exposes tracking information for the HMD and two controllers, and
renders to a device using raylib's existing stereo rendering facilities. 

See the `examples/` directory for some work-in-progress examples using the fork.

## Getting Started

### Developing raylib-openvr

- Using [vcpkg](https://github.com/microsoft/vcpkg), install `openvr`
- Clone this repository recursively
- Work on examples in `examples/` and raylib code in the submodule

### Using raylib-openvr (CMake)

- Understand that this project is still a work-in-progress!
- Using [vcpkg](https://github.com/microsoft/vcpkg), install `openvr`
- Clone or submodule [dhsavell/raylib](https://github.com/dhsavell/raylib/tree/openvr) into your project 
    - Ensure you have checked out the `openvr` branch
    - Using the source is recommended as the project is constantly being updated
- Use `add_subdirectory` to include it in your build
    ```cmake
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory("raylib")
    target_link_libraries(yourproject raylib)  
    ```