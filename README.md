# nina-chess

Cute engin with everything other than good play. Would be fast if it didn't generate moves on leaf nodes, as moves in a given position are used for evaluation (lolz that's so cool).

# license

custom license written by AI that basically says you can use it however you want, but you cannot use it for commercial purposes if your purpose generates over 1M in revenue annually. a statement against open source projects letting themselves be royally f*ked by amazon, more than anything practical. 

## what's in here

### search

iterative deepening alpha-beta. that's it. more to come !!!!! (maybe)

there's a transposition table

draw scores get a tiny random variance (+-3.5) so the engine doesn't just shuffle pieces around like a bored cat. (stolen from ethereal)

### evaluation

PSQT (piece-square tables) with 25 bitboard features fed into an accumulator with tanh activation. the features include all the piece bitboards but also stuff like pin masks, check masks, attacked squares, and per-piece mobility. 

incremental updates in accumulator blatantly stolen from NNUE architecture, everything else is original

### move generation

fully legal moves via bitboards. PEXT (BMI2) for sliding piece attacks with a software fallback for CPUs that don't have it. pin-aware, check-aware, the whole deal. idea stolen from Gigantua move enumerator, implementation fully original

moves also produce auxiliary data (pin masks, check masks, attacked squares) which feeds directly into evaluation. does it improve strength? we will see lol

### SIMD

AVX-512, AVX2, and SSE3 support. the neural net forward pass, horizontal sums, and activation functions all use SIMD. builds are parameterized by SIMD arch so you pick what your CPU supports.  

avx code is super cool though!! it's fast

### transposition table

yes

### time management

no

### UCI

standard UCI protocol. `go`, `stop`, `position`, `setoption`, all the usual suspects. configurable hash size and weights file (weights file ignored).

### game generation

self-play, has a progress bar with ETA because watching numbers go up is important.

the opening book is a custom binary format with positions indexed by ply, only used in self-play. one day this will be used to train something

### build targets

five of them, all sharing the same source files with preprocessor flags controlling which `main()` compiles:
- **nina-chess**: the UCI engine
- **test**: perft + search tests
- **bench**: perft + search benchmarks
- **debug**: same as release but with debug assertions and no optimization for debuk
- **gamegen**: self-play game generation

### things that are notably missing

everything else that exists, one day maybe perhaps !!

## building

```sh
just build                          # nina-chess, AVX2
just build test                     # test target
just build nina-chess AVX512        # specific SIMD arch
```

or cmake directly:
```sh
cmake -B .artifacts/cmake/AVX2 -DSIMD_ARCH=AVX2
cmake --build .artifacts/cmake/AVX2 --target nina-chess
```

or open `nina-chess.sln` in Visual Studio

supports MSVC, GCC, and Clang. C++26.
