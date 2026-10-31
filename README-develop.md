# 开发

## 1 环境搭建

### Windows

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

### Ubuntu

TODO

## 2 模块设计

### 总体设计思路

飞行射击游戏的数据在游玩过程中基本是静态的，也就是占用的堆内存总量基本不变，不存在运行时的动态内存分配。因此在初始化时，所有堆内存就应该被分配好。

堆内存也就是各模块的静态全局变量，它们在各模块的 Init 函数中被初始化。初始化流程包括空间的分配和初始数据的填充。数据的来源是 lua 配置文件，因此几乎所有模块都需要包含 lua 模块。

模块之间通过对外提供接口进行信息交换。

### 输入模块（Input 模块）

Input 模块对外提供获取输入接口 GetInput ，该接口可以指定按键、设备和触发方式，从而获取所需输入。

### 图形模块（Graphic 模块）

