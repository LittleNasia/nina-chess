cmake_minimum_required(VERSION 3.13)
project(nina-chess)
set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")
set(CMAKE_CXX_STANDARD 26)
set(SOURCE_FILES ./nina-chess/bench_main.cpp
        ./nina-chess/nina-chess.cpp
        ./nina-chess/test_main.cpp
        ./nina-chess/uci.cpp )
add_executable(nina-chess ${SOURCE_FILES})
add_executable(bench ${SOURCE_FILES})
add_executable(test ${SOURCE_FILES})
add_executable(debug ${SOURCE_FILES})

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

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(ALL_TARGET_GCC_FLAGS -mavx2 -mbmi -mbmi2 -static -Wno-return-type -Wno-switch)
    set(RELEASE_GCC_FLAGS ${ALL_TARGET_GCC_FLAGS} -Ofast -finline-limit=500000 )

    target_compile_options(nina-chess PRIVATE ${RELEASE_GCC_FLAGS})
    target_compile_options(bench PRIVATE ${RELEASE_GCC_FLAGS})
    target_compile_options(test PRIVATE ${RELEASE_GCC_FLAGS})
    target_compile_options(debug PRIVATE ${ALL_TARGET_GCC_FLAGS})
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(ALL_TARGET_CLANG_FLAGS -mavx2 -mbmi -mbmi2 -static -Wno-return-type -Wno-switch)
    set(RELEASE_CLANG_FLAGS ${ALL_TARGET_CLANG_FLAGS} -O3 -ffast-math)


    target_compile_options(nina-chess PRIVATE ${RELEASE_CLANG_FLAGS})
    target_compile_options(bench PRIVATE ${RELEASE_CLANG_FLAGS})
    target_compile_options(test PRIVATE ${RELEASE_CLANG_FLAGS})
    target_compile_options(debug PRIVATE ${ALL_TARGET_CLANG_FLAGS})
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

endif()
