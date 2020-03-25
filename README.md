# Odin - A Vulkan-based Path Tracer

Odin is a path tracer that is written using the Vulkan API and compute shaders.
Odin has been tested on an NVIDIA Geforce GTX 970 and Intel Integrated graphics GPUs.

## Dependencies
Make sure you have CMake, the Vulkan API, and Boost's `boost::program_options` installed on your system.
It is also important to have a compiler that supports c++17 to be able to compile Odin.

## Installation
1. Clone the repository - `git clone --recursive https://github.com/tstullich/odin.git`
2. Create a build directory and build
```
cd odin
mkdir build
cd build
cmake ..
make install
```

Odin should then be installed in the `bin` directory. Type `odin --help` for more information.

## TODOs
Below is an unordered list of planned features and improvements for Odin
1. Rewrite BVH construction and traversal logic to use [SAH k-d tree](https://www.researchgate.net/publication/232652917_On_Building_Fast_kd-trees_for_Ray_Tracing_and_on_Doing_that_in_ON_log_N)
2. Implement Multiple Importance Sampling
3. Improve the compute and graphics pipelines to do batched rendering
4. Update the camera model to use a view matrix
5. Expand material support
6. Add a GUI for tuning scene parameters at runtime (Dear Imgui)
7. Move Vulkan header to use [Vulkan-Hpp](https://github.com/KhronosGroup/Vulkan-Hpp)