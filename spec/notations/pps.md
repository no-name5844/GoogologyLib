# PPS (Parented Predecessor Sequence — 标记父项型 / marked_parent)

- **Family / 族**: ordinal (序数记号)
- **Subfamily / 子族**: sequence (自然数序列记号)
- **Style / 风格**: marked_parent (标记父项型)
- **Creator / 创造者**: `318\`4`（PPS 系列）
- **Compare / 比较**: DEFINED — lexicographic on the sequence values (字典序)，仅同变体可比
- **Supported ops / 支持运算**: FromString, ToString, Expand, ExpandTo, Normalize, Compare, Successor
- **`normalize()` / `isSuccessor()`**: 非虚的 `OrdinalNotation` 基类（继承 `Notation`）函数（所有序数序列记号共用同一份实现，仅由 `baseVal_` 区分；不属求值类）
- **Evaluate / 求值**: **not provided / 不提供**（一律不求值 / never evaluated）
- **Known facts / 已知结论**: 良序极限 = ζ₀；PPS 存在无穷降链问题；PPS2 为失败的弱化修复；PPS4 旨在删除无穷降链。PPS3 / ePPS4 / sPPS4 资料缺失（N/A），不实现。

> PPS 系列占据 taxonomy 中的 `ordinal/sequence/marked_parent/` 槽。与阶差型
> (difference) PrSS 的差异仅在末项替换规则：PrSS 无条件把末项换为坏根值，
> PPS 仅当坏根与末项之间存在值等于坏根值的项时才换，否则末项减 1。
> The PPS family occupies the `ordinal/sequence/marked_parent/` taxonomy slot.
> Unlike difference-type PrSS (which unconditionally replaces the last term
> with the bad-root value), PPS replaces only when a term equal to the bad
> root's value sits between the bad root and the last term; otherwise the
> last term is decremented.

## 实现范围 / Implemented variants

| 记号 | 库类 | 注册名 | 版本 | 说明 |
|---|---|---|---|---|
| PPS (PPS1) | `Pps1` | `pps1` | `1` | 原始 PPS |
| PPS2 | `Pps2` | `pps2` | `2` | 弱化修复（掩耳盗铃版） |
| PPS3 | — | — | — | **N/A**（资料缺失，不实现） |
| PPS4 | `Pps4` | `pps4` | `4` | 削弱强展开，尝试删除无穷降链 |
| wPPS4 | `WPps4` | `wpps4` | `4w` | weak：强展开判定收紧 |
| tPPS4 | `TPps4` | `tpps4` | `4t` | third：强展开复制加阶差 |
| fPPS4 | `FPps4` | `fpps4` | `4f` | fourth：强展开首复制为坏根列标 |
| ePPS4 / sPPS4 | — | — | — | **N/A**（资料缺失，不实现） |

## Output format (LaTeX) / 输出格式（LaTeX）

`to_string()` / `print()` / `operator<<` emit **LaTeX**, e.g.
`(0, 1, 0, 3)`.
The library never computes a numeric/ordinal value. `expand(m)` /
`expand_to(M)` / `reduce()` **rewrite the internal sequence and return the
notation object itself**, never a string; call `to_string()` to render LaTeX
on demand. `string_to_it()` / `operator>>` accept **ASCII** `(0, 1, 0, 3)`
input.

## Definition / 定义

A sequence \(A = (a_1, a_2, \dots, a_n)\)，首项是第 1 项 (1-based columns)。
极限表达式 (limit expression): \(0, 1, 2, 3, 4, 5, \dots\)

### Auxiliary / 辅助

- 末项值 \(x = a_n\)；末项列标 \(y = n\)。
- **坏根 (bad root)** = 第 \(x\) 项（列标等于末项值的那一项，1-based）。
- 坏根值 \(b = a_x\)。
- 尾长 \(L = y - x\)。

### Expansion / 展开（公共骨架，全部变体一致）

1. 末项为 0（后继序数）：`expand` 一步 = 去掉末项。
2. 替换末项（按变体，见下）。
3. 复制：对任意 \(i > x\)，第 \(i+L\) 项由第 \(i\) 项确定：
   \[
   a_{i+L} = \begin{cases} a_i + L & a_i \ge x \\ a_i & a_i < x \end{cases}
   \]
4. **基本列 \([n]\) = 展开到第 \(y + nL - 1\) 项**。

> **Indexing note / 索引说明.** 资料的基本列下标 \(n\) 从 1 起（[1] = 最小项）。
> 库按项目约定（所有记号基本列索引统一 0 起）映射：`expand(k) = 基本列[k+1]`
> = `expandLen((k+1)L - 1)`，其中 `expandLen(A, M)` = 替换末项 + 追加 \(M\) 项。
> `expand_to(M) = expandLen(A, M)`。追加项按上述复制规则从**运行中（已增长的）
> 序列**取值（源位置 \(i = p + y - L\)，\(p\) 为新项序号）。

### 变体规则 / Variant rules（仅末项替换 / 强展开复制不同）

**PPS1** — 末项：
\[
\text{末项} = \begin{cases}
b & \exists k,\ x<k<y,\ a_k = b \\
x - 1 & \text{否则}
\end{cases}
\]

**PPS2** — 存在值 \(= b\) 的项（取最右，列标 \(k\)）时逐项比较：
对 \(j = 0, 1, 2, \dots\) 比较 \(a_{k+j}\) 与 \(a_{c+j}\)（\(c = x\) 为坏根列标），
首个不同处**前者更大**才把末项换为 \(b\)；更小 / 越界平局则末项减 1；
全等平局则末项减 1（防御性取法）。

**PPS4** — 弱/强展开二分：
- 弱展开：\(\exists k,\ x<k<y,\ a_k = b\) → 末项换为 \(b\)。
- 强展开：否则，在**第 \(b\) 列与第 \(x\) 列之间（都不含）** 找最右侧值
  \(= b\) 的项，末项换为该**列标**；找不到 → 等同弱展开（换为 \(b\)）。

**wPPS4** — 注记：把 PPS4 的强展开判定由“≤ b”收紧为“= b”。
PPS4 正文即作 “= b”，按字面实现后二者强展开行为一致；若日后确认 PPS4
应为 “≤ b”，仅需放宽 PPS4 的强查找谓词。

**tPPS4** — wPPS4 + **强展开**时由末项复制出的项加阶差（阶差 = 复制宽度 \(L\)）：
源位置 \(i \equiv y \pmod L\) 的追加项值 = 源值 + \(L\)（弱展开时不变）。

**fPPS4** — tPPS4 + **强展开**时末项**第一次**复制（源位置 \(i = y\)）=
坏根列标 \(x\)；后续同 tPPS4（+\(L\)）。

## Expansion trace / 展开轨迹

```
PPS1 (0,1,2):        x=2, y=3, 坏根=第2项=1, b=1, L=1；坏根与末项间无值=1 → 减1
  expand(0) = (0, 1, 1)
  expand(1) = (0, 1, 1, 1)
  expand(2) = (0, 1, 1, 1, 1)

PPS1 (0,1,0,3):      x=3, y=4, 坏根=第3项=0, b=0, L=1；坏根与末项间为空 → 减1
  expand(0) = (0, 1, 0, 2)
  expand(1) = (0, 1, 0, 2, 2)

PPS1 (0,1,0,1,2):    x=2, y=5, 坏根=第2项=1, b=1, L=3；存在值=1（第4项）→ 换 b
  expand(0) = (0, 1, 0, 1, 1, 0, 1)

PPS1 vs PPS2 (0,2,4,4,2,3):  x=3, b=4；PPS1 存在值=4 → 换 4；
  PPS2 比较 a_{4+j} vs a_{3+j}：j=1 处 2<4 → 减 1
  expand(0): PPS1 = (0, 2, 4, 4, 2, 4, 7, 2)   PPS2 = (0, 2, 4, 4, 2, 2, 7, 2)

PPS4 (0,1,0,2,2,3): x=3, b=0；坏根与末项间无值=0 → 强展开；第0列与第3列
  之间最右值=0 是第1列 → 末项换为 1
  expand(0) = (0, 1, 0, 2, 2, 1, 2, 2)    （PPS1 同例 = (0,1,0,2,2,2,2,2)）

tPPS4 (0,1,0,2,2,3) expand(1): 强展开；第3个新项源 = 末项（i=6）→ +L=+3
  = (0, 1, 0, 2, 2, 1, 2, 2, 4, 2, 2)     （PPS4 同例末项复制 = 1）
fPPS4 (0,1,0,2,2,3) expand(1): 末项第一次复制 = 坏根列标 3
  = (0, 1, 0, 2, 2, 1, 2, 2, 3, 2, 2)
```

## String syntax (parser) / 字符串语法

- Input via `string_to_it()` / `operator>>`: ASCII tuple / ASCII 元组
  `(0, 1, 0, 3)` (whitespace optional / 空格可选).
- `to_string()` / `print()` / `operator<<` output LaTeX / 输出 LaTeX:
  `(0, 1, 0, 3)`.

## API mapping / 接口映射

| Definition / 定义 | Library call / 库调用 | Returns / 返回 |
|---|---|---|
| `expand(A, k)`（基本列[k+1]，0 索引） | `expand(k)` | `Pps&` (self, rewritten) |
| `expandLen(A, M)`（替换末项 + 追加 M 项） | `expand_to(M)` | `Pps&` (self, rewritten) |
| 基本列第 k 项（不修改 A） | `A[k]` | 变体对象（拷贝） |
| render / 渲染 | `to_string()` | LaTeX string |
| parse / 解析 | `string_to_it()` | — |

`reduce()` repeatedly applies `expand(1)` until the symbolic form is stable.

## Comparison / 比较

`compare()` is **lexicographic (字典序)** on the sequence values, valid when
both operands are in standard form (consistent with Prss). The master limit
expression compares as the supremum. **Cross-variant** comparisons (e.g.
PPS1 vs PPS2) throw `NotComparable` — each variant is its own notation system.
Cross-family comparisons (e.g. PPS vs a large-number notation) also throw
`NotComparable`.

## Standard form (normalize) / 标准型

PPS supports `normalize()` / `is_standard()` via the universal §12 engine:
roots = the variant's master limit \(0,1,2,3,\dots\) (`limit(n) = (0,1,...,n-1)`),
`LIMIT.expand(m) = limit(m)`. ⚠ The well-ordering limit is ζ₀ (external);
the reachable standard forms are a proper subset (the PPS 良序部分).
