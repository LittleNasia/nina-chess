cmake_minimum_required(VERSION 3.13)
project(nina-chess)
set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")
set(CMAKE_CXX_STANDARD 26)

set(SIMD_ARCH "AVX2" CACHE STRING "SIMD architecture: SSE3, AVX2, or AVX512")
set_property(CACHE SIMD_ARCH PROPERTY STRINGS SSE3 AVX2 AVX512)

set(SOURCE_FILES ./nina-chess/SourceFiles/bench_main.cpp
        ./nina-chess/SourceFiles/nina-chess.cpp
        ./nina-chess/SourceFiles/test_main.cpp
        ./nina-chess/SourceFiles/uci.cpp
        ./nina-chess/SourceFiles/game_generation_main.cpp )
include_directories(${CMAKE_SOURCE_DIR}/nina-chess)

add_executable(nina-chess ${SOURCE_FILES})
add_executable(bench ${SOURCE_FILES})
add_executable(test ${SOURCE_FILES})
add_executable(debug ${SOURCE_FILES})
add_executable(gamegen ${SOURCE_FILES})

target_compile_definitions(nina-chess PUBLIC
        ADD_DEBUG_CODE=false)
target_compile_definitions(bench PUBLIC
        ADD_DEBUG_CODE=false
        _BENCH
        _PERFTNODES=1000000)
target_compile_definitions(test PUBLIC
        ADD_DEBUG_CODE=false
        _TEST
        _PERFTNODES=18446744073709551615ULL)
target_compile_definitions(debug PUBLIC
        ADD_DEBUG_CODE=true)
target_compile_definitions(gamegen PUBLIC
        ADD_DEBUG_CODE=false
        _GAMEGEN)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(SIMD_ARCH STREQUAL "SSE3")
        set(SIMD_FLAGS -msse3)
    elseif(SIMD_ARCH STREQUAL "AVX2")
        set(SIMD_FLAGS -mavx2 -mbmi -mbmi2)
    elseif(SIMD_ARCH STREQUAL "AVX512")
        set(SIMD_FLAGS -mavx512f -mbmi -mbmi2)
    endif()

    set(ALL_TARGET_GCC_FLAGS ${SIMD_FLAGS} -static -Wno-return-type -Wno-switch)
    set(RELEASE_GCC_FLAGS ${ALL_TARGET_GCC_FLAGS} -Ofast -finline-limit=500000 )

    target_compile_options(nina-chess PRIVATE ${RELEASE_GCC_FLAGS})
    target_compile_options(bench PRIVATE ${RELEASE_GCC_FLAGS})
    target_compile_options(test PRIVATE ${RELEASE_GCC_FLAGS})
    target_compile_options(debug PRIVATE ${ALL_TARGET_GCC_FLAGS})
    target_compile_options(gamegen PRIVATE ${RELEASE_GCC_FLAGS})
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(SIMD_ARCH STREQUAL "SSE3")
        set(SIMD_FLAGS -msse3)
    elseif(SIMD_ARCH STREQUAL "AVX2")
        set(SIMD_FLAGS -mavx2 -mbmi -mbmi2)
    elseif(SIMD_ARCH STREQUAL "AVX512")
        set(SIMD_FLAGS -mavx512f -mbmi -mbmi2)
    endif()

    set(ALL_TARGET_CLANG_FLAGS ${SIMD_FLAGS} -static -Wno-return-type -Wno-switch)
    set(RELEASE_CLANG_FLAGS ${ALL_TARGET_CLANG_FLAGS} -O3 -ffast-math)


    target_compile_options(nina-chess PRIVATE ${RELEASE_CLANG_FLAGS})
    target_compile_options(bench PRIVATE ${RELEASE_CLANG_FLAGS})
    target_compile_options(test PRIVATE ${RELEASE_CLANG_FLAGS})
    target_compile_options(debug PRIVATE ${ALL_TARGET_CLANG_FLAGS})
    target_compile_options(gamegen PRIVATE ${RELEASE_CLANG_FLAGS})
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(SIMD_ARCH STREQUAL "AVX2")
        set(SIMD_FLAGS /arch:AVX2)
    elseif(SIMD_ARCH STREQUAL "AVX512")
        set(SIMD_FLAGS /arch:AVX512)
    else()
        set(SIMD_FLAGS "")
    endif()

    target_compile_options(nina-chess PRIVATE ${SIMD_FLAGS})
    target_compile_options(bench PRIVATE ${SIMD_FLAGS})
    target_compile_options(test PRIVATE ${SIMD_FLAGS})
    target_compile_options(debug PRIVATE ${SIMD_FLAGS})
    target_compile_options(gamegen PRIVATE ${SIMD_FLAGS})
endif()
