# Grid

![Grid](assets/logo.svg)

Block puzzle game: place pieces from a tray of three onto an 8x8 board and clear full rows and columns.

## Dependencies

- C++20 compiler
- CMake 3.20 or newer
- SDL3
- SDL3_ttf

## Build

```
cmake -S . -B build
cmake --build build
ctest --test-dir build
./build/grid
```

## License

GPL-3.0-only. The bundled Geologica font is licensed under the SIL Open Font License 1.1, see assets/OFL.txt.
