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
