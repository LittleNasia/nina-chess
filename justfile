set windows-shell := ["pwsh", "-NoProfile", "-Command"]

# Build nina-chess with CMake
# Usage: just build [target] [arch]

default_target := "nina-chess"
default_arch := "AVX2"

build target=default_target arch=default_arch:
    cmake -B .artifacts/cmake/{{arch}} -DSIMD_ARCH={{arch}}
    cmake --build .artifacts/cmake/{{arch}} --target {{target}}
