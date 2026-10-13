# 开发

## 环境搭建

# 环境搭建

## Windows

使用 MYSY2 作为包管理器。

清华源 MSYS2: https://mirrors.tuna.tsinghua.edu.cn/help/msys2/

toolchain

```bash
pacman -S mingw-w64-ucrt-x86_64-toolchain
```

cmake

```bash
pacman -S \
mingw-w64-ucrt-x86_64-ccmake \
mingw-w64-ucrt-x86_64-cmake \
mingw-w64-ucrt-x86_64-cmake-cmcldeps \
mingw-w64-ucrt-x86_64-cmake-docs \
mingw-w64-ucrt-x86_64-cmake-gui
```

sdl3

```bash
pacman -S \
mingw-w64-ucrt-x86_64-sdl3 \
mingw-w64-ucrt-x86_64-sdl3-ttf \
mingw-w64-ucrt-x86_64-sdl3-image \
mingw-w64-ucrt-x86_64-sdl3-mixer
```

lua

```bash
pacman -S mingw-w64-ucrt-x86_64-lua54
```

