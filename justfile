set windows-shell := ["pwsh", "-NoProfile", "-Command"]

default_target := "nina-chess"
default_arch := "AVX2"

# Print usage
default:
    @echo "Usage: just build [target] [arch]"
    @echo ""
    @echo "Targets: nina-chess (default), test, bench, debug, gamegen"
    @echo "Architectures: SSE3, AVX2 (default), AVX512"
    @echo ""
    @echo "Examples:"
    @echo "  just build              # builds nina-chess with AVX2"
    @echo "  just build test         # builds test target with AVX2"
    @echo "  just build bench SSE3   # builds bench target with SSE3"
    @echo ""
    @echo "Output: .artifacts/cmake/<arch>/<target>/<target>.exe"

# Build a target with CMake. Targets: nina-chess, test, bench, debug, gamegen. Architectures: SSE3, AVX2, AVX512
build target=default_target arch=default_arch:
    cmake -B .artifacts/cmake/{{arch}} -DSIMD_ARCH={{arch}}
    cmake --build .artifacts/cmake/{{arch}} --target {{target}}
