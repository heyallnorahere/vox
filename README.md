# vox

Voxel engine in C++.

## Dependencies

Required dependencies:
- [CMake](https://cmake.org/) >= 3.21
- A C++20 toolchain
- The Vulkan runtime >= 1.2.0
- [Git](https://git-scm.com/downloads)

While not required, it is recommended to install the [Vulkan SDK](https://vulkan.lunarg.com/).

## Building instructions

From your preferred source directory:
```bash
# cloning
git clone --recursive git@github.com:heyallnorahere/vox.git
cd vox

# default (release)
CONFIG=default

# ...or single-config generators, debug
CONFIG=debug

# configure & build
cmake --preset $CONFIG
cmake --build --preset $CONFIG
```

## Running

From the repository root directory:
```bash
cd vox

# For release builds
CONFIG=Release

# ...or for debug builds
CONFIG=Debug

../build/vox/$CONFIG/vox
```