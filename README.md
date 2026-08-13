# GoogologyLib — Lean4 branch

多语言记号库的 **Lean4 分支**（参考实现为 C++ 分支；其余分支：C / C# / java / python / Rust）。

## ⚠ 依赖：mathlib

本分支**依赖 mathlib**（Lean 官方数学库），版本锁定见 `lakefile.lean`
（`require mathlib ... @ "v4.30.0-rc2"`）。这是与其它语言分支（无外部依赖）
的关键差异：

- 序数语义层（`googology/core/Ordinal.lean` 的 `MOrdinal := Ordinal`、
  `googology/core/OrdinalSemantics.lean` 的 `sSup`）直接使用
  `Mathlib.SetTheory.Ordinal.*` 的**真序数**（`Ordinal`）；
- 记号层（`Prss.lean` / `Pps.lean` / `Bms.lean` / `EpsilonSS.lean` /
  `WeakVeblen.lean` / `Ns.lean` / `Conway.lean` / `Knuth.lean`）**不依赖**
  mathlib，仅使用核心库。

构建：

```bash
lake build        # 首次会拉取并编译 mathlib（耗时较长）
```

## 分支现状 / Status

- 记号实现：Knuth / Conway / Prss / EpsilonSS / WeakVeblen / Bms / Ns / PPS
- 序数语义（接轨 mathlib Ordinal）：`OrdinalSemantics.lean`（`HasOrdSemantics`
  typeclass + `ordinalOf` 三分支语义）+ `OrdSemantics.lean`（Prss/Pps 实例）
- ⚠ 分支既有文件（`Prss.lean` 等）在 lean 4.30.0-rc2 下存在兼容问题，
  整体 `lake build` 需先修复（见 `core/OrdinalSemantics.lean` 提交记录）。
