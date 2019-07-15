# Odin - A Vulkan-based Path Tracer

Odin is going to be a path tracer that will be written using the Vulkan API using compute shaders

## Dependencies
Make sure you have CMake, the Vulkan API, and Boost's `boost::program_options` installed on your system

## Installation
1. Clone the repository
2. Initialize the submodules - `git submodule init && git submodule update` 
4. Create a build directory
```
mkdir build
cd build
cmake ..
make install
```

Odin should then be installed in the `bin` directory
