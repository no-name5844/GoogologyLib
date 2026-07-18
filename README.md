# GoogologyLib — 多语言记号收集库 / Multi-language Notation Collection Library

> 中文 | English
>
> 本仓库以多语言分支（C / C++ / java / python / Lean4）分别实现同一套逻辑骨架；
> `master` 存放与语言无关的规格 `spec/` 与黄金测试向量 `tests/vectors/`。
>
> This repository implements one logical skeleton across multiple language branches
> (C / C++ / java / python / Lean4). `master` holds the language-neutral `spec/`
> and golden `tests/vectors/`.

> **当前版本 / Current version**: `v0.1.1` — 发布以 git tag 提供，见
> [Releases](https://github.com/no-name5844/GoogologyLib/releases)。

---

## 简介 / Introduction

**中文** — GoogologyLib 收集各类"记号"（notation），并为其提供**展开 / 重写规则**与
（在定义良好的前提下）**大小比较**。库本身**不直接计算记号的数值**，只说明"该如何计算"：
通过 `expand` / `reduce` 将记号按其自身规则逐步重写，结果始终停留在记号自己的符号形式中，
绝不坍缩为裸整数。

**English** — GoogologyLib collects various *notations* and supplies their **expansion /
rewriting rules** and (where well-defined) **comparison**. The library **does not compute the
numeric value** of a notation; it only states *how to compute*: `expand` / `reduce` rewrite the
notation step by step under its own rules, and the result always stays in the notation's own
symbolic form — never collapsed to a bare integer.

## 核心原则 / Key Principles

1. **序数记号 ≠ 大数记号 / Ordinal notations ≠ large-number notations**
   二者表示不同对象，不可跨族比较。序数记号族内可比较；大数记号比较通常未定义。
   The two denote different objects and must not be compared across families. Ordinals compare
   within family; large-number comparison is generally undefined.
2. **不计算数值 / No numeric evaluation**
   库从不把记号求值成数字；即使是普通算术（如 `3^3`）也保留符号形式。
   The library never evaluates a notation to a number; even ordinary arithmetic (e.g. `3^3`)
   is kept symbolic.
3. **能力位模型 / Capability model**
   接口全量声明所有运算，能力位标出实现了哪些；未实现的运算调用时抛 `UnsupportedOperation`。
   The interface declares every operation; a capability bitset marks which are implemented;
   calling an unsupported op throws `UnsupportedOperation`.

## 使用政策与免责声明 / Usage Policy & Disclaimer

- **大数记号拒绝求值型运算**：Knuth ↑ / Conway → 等记号一旦发生"减一 / 取前驱 / 左减"
  这类依赖先求出数值的运算，库**直接拒绝、不计算**（见 `doc/design.md` §10）。
  Large-number notations **refuse** evaluation-style ops (minus-one / predecessor /
  left-subtraction): the library refuses outright and **never computes** (see `doc/design.md` §10).
- **仅供研究参考**：本库提供的展开 / 重写 / 比较**仅供研究参考**，不能在直接分析时
  当作代替的计算器。For research reference only — not a substitute calculator.
- **学习以定义为准**：研究学习应基于记号的数值含义与相关定义本身，不能单纯依靠本库。
  Learn from definitions and related materials; do not rely on this library alone.

## 记号族 / Notation Families

- `ordinal/` 序数记号 — `compare` 定义良好（序数序）。下设 `sequence/`（自然数序列记号）：
  `sequence/difference/`（阶差型，如 PPS）、`sequence/marked_parent/`（标记父向型，如 Y）。
- `number/` 大数记号 — `compare` 未定义（抛 `NotComparable`）。如 Knuth ↑、Conway →、BEAF。
- `hierarchy/` 桥接族 — 把序数映到函数（FGH / Hardy / SHG），规划中。

完整分类与契约见 `doc/design.md`。 / See `doc/design.md` for the full taxonomy and contract.

## 本分支实现 / This branch (C++) implements

| 记号 notation | 文件 file | 支持的运算 ops (target) |
|---|---|---|
| Knuth up-arrow (高德纳箭头) | `include/googology/notations/number/knuth/` | Parse, Serialize, Expand, ExpandTo |
| Conway chained arrow (康威链式箭头) | `include/googology/notations/number/conway/` | Parse, Serialize, Expand, ExpandTo |

两者均**不提供** `Compare`（大数记号比较未定义，抛 `NotComparable`）。
Neither provides `Compare` (large-number comparison undefined → `NotComparable`).

## 构建 / Build

```bash
cmake -S . -B build
cmake --build build
./build/demo
ctest --test-dir build
```

默认整数类型 `int64_t`（无外部依赖），仅用于解析参数 / 展开索引，不作数值计算。
Default integer type is `int64_t` (no external deps), used only for parse parameters and
expansion indices — never for numeric evaluation.

## 获取某一版本（无需克隆整库）/ Fetch a single version (no full clone)

发布以 **git tag** 形式提供。使用者只需取某一个版本快照，无需克隆整个仓库历史：

**中文**
- 浅克隆指定版本：`git clone --depth 1 --branch v0.1.1 https://github.com/no-name5844/GoogologyLib.git`
- 或在 GitHub Releases 页下载该版本的源码压缩包（zip / tar.gz），解压即用。
- **预编译库**：Windows x64 用户可直接用 `lib/windows-x64/` 下的 `libgoogology.a` /
  `libgoogology.dll` / `libgoogology.dll.a` 链接，无需自行编译；其他平台请运行
  `scripts/build_lib.sh` 自行构建（产物与平台 / 编译器强相关）。
- 升级版本：仅改 tag 名（如 `v0.2.0`）重新浅克隆 / 重新下载即可。

**English**
- Shallow-clone a version: `git clone --depth 1 --branch v0.1.1 https://github.com/no-name5844/GoogologyLib.git`
- Or download the source archive (zip / tar.gz) of that tag from GitHub Releases.
- **Prebuilt libs**: Windows x64 users can link `lib/windows-x64/libgoogology.{a,dll,dll.a}`
  directly; other platforms run `scripts/build_lib.sh` to build (artifacts are platform/compiler specific).
- Upgrade: change the tag name and re-clone / re-download.

## 快速示例 / Quick example

```cpp
#include "googology/notations/number/knuth/Knuth.hpp"
using namespace googology::number;
Knuth k("2 ^^ 3");
std::cout << k.expand(1);     // one rewrite step -> "2 ^ (2 ^^ 2)"
// k.compare(other);          // throws NotComparable (large-number comparison undefined)
```

## 文档状态 / Doc status

> 本文件对应 **v0.1.1**（C++ 参考实现分支）。代码已对齐设计：记号一律不求值，`expand` /
> `reduce` 返回符号化重写轨迹；能力位标出已实现运算。
>
> This document matches **v0.1.1** (the C++ reference branch). The code is aligned with the
> design: notations are never evaluated; `expand` / `reduce` return symbolic traces; the
> capability bitset marks which ops are implemented.

## 开源协议 / License

本项目以 **MIT License** 发布 —— 完全开源、OSI 批准。

- **中文**：任何人可自由使用、复制、修改、合并、发布、再许可及销售本软件，
  只需在副本中保留上述版权声明与本许可声明。本软件按「原样」提供，不附任何担保。
- **English**: Distributed under the **MIT License** (fully open source, OSI-approved).
  Anyone may use, copy, modify, merge, publish, sublicense, and/or sell copies,
  provided the copyright notice and this permission notice are included. The software
  is provided "AS IS", without warranty.

完整文本见 [LICENSE](./LICENSE)。各语言分支的源文件可含 SPDX 标识
`SPDX-License-Identifier: MIT`。
