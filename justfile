# Build nina-chess with CMake
# Usage: just build [target] [arch]

default_target := "nina-chess"
default_arch := "AVX2"

build target=default_target arch=default_arch:
    cmake -B build/{{arch}} -DSIMD_ARCH={{arch}}
    cmake --build build/{{arch}} --target {{target}}
