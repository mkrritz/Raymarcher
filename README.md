# Raymarcher

A minimal OpenGL fragment shader raymarcher.

## Requirements

- [MSYS2](https://www.msys2.org) with the **UCRT64** environment
- `mingw-w64-ucrt-x86_64-gcc`
- `mingw-w64-ucrt-x86_64-make`
- `mingw-w64-ucrt-x86_64-pkg-config`
- `mingw-w64-ucrt-x86_64-glfw`
- `mingw-w64-ucrt-x86_64-glew`
- `mingw-w64-ucrt-x86_64-glm`

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-pkg-config mingw-w64-ucrt-x86_64-glfw mingw-w64-ucrt-x86_64-glew mingw-w64-ucrt-x86_64-glm
```

## Compile & Run

```bash
mingw32-make
Binaries/Raymarcher.exe
```

