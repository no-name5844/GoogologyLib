# 预编译库 / Prebuilt libraries

本目录存放**已编译**的库文件，随版本一起发布，便于使用者直接链接，无需自行编译。

This directory holds **prebuilt** library files, shipped with each release so
consumers can link without building from source.

## 当前提供 / Provided here

`windows-x64/` — 由 MinGW-w64 g++（C++17, `-O2`）在 Windows x64 上构建：

- `libgoogology.a`      — 静态库 (static archive)
- `libgoogology.dll`    — 动态库 (shared library)
- `libgoogology.dll.a`  — 动态库导入库 (import lib, for MinGW linking)

链接示例 / Linking example:

```bash
g++ -std=c++17 -Iinclude your_app.cpp -Llib/windows-x64 -lgoogology -o your_app
```

## 其他平台 / Other platforms

Linux / macOS 用户请自行构建（库产物与平台、编译器强相关）：

    bash scripts/build_lib.sh

将生成 `build/lib/libgoogology.a` 与 `build/lib/libgoogology.so`（或 `.dylib`）。

## 版本对应 / Version correspondence

每个 `lib/` 快照对应一个 git tag（如 `v0.1.0`）。升级时取对应 tag 的库文件即可，
无需克隆整库历史。
