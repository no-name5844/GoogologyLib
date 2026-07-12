# weak-Veblen-like (zahin 的类弱 Veblen 记号)

- **Family / 族**: ordinal (序数记号)
- **Subfamily / 子族**: veblen (经典序数 / classic ordinal)
- **Style / 风格**: weak-like
- **Compare / 比较**: **已实现（逐字转录 §1.3 的 is_equal / is_greater / compare）**；见第 5 节 C4 / C5。
- **Supported ops / 支持运算**: FromString, ToString, Expand
- **Evaluate / 求值**: **not provided / 不提供**（一律不求值 / never evaluated）

> 本记号对应 `study/notations/weak-Veblen-like notation.md`，源码位于
> `include/googology/notations/ordinal/veblen/weakveblen/`。
> Source: `study/notations/weak-Veblen-like notation.md`; code in
> `include/googology/notations/ordinal/veblen/weakveblen/`.

---

## 1. 文章原文（逐字 / verbatim）

# 1 zahin's weak-Veblen-like notation (zahin的类弱veblen记号) by zahin

## 1.1 定义
是不封闭，是序数记号中的序列记号。极限表达式为 \((0),(1@(0)),(1@(1@(0))),\dots\)
该记号在标准型下满足字典序比较
对于 \( A= (a_1@b_1,a_2@b_2,\dots,a_n@b_n)\) 一定满足：
1. \(a_i , b_i \in On.\)
2. \(b_i > b_{i+1}\)

## 1.2 展开

### 1.2.1 expand 函数
对于 \(A=(0)\)
\[
\mathrm{expand}(A,n)=n
\]

对于 \(A=(0@b_1,\#)\)
\[
\mathrm{expand}(A,n)=(\#)
\]

对于 \(A=(\#,(a+1)@0)\)
\[
\mathrm{expand}(A,n)=
\begin{cases}
(\#,a@0)+1 & n=0\\
(\#,a@0)^{\mathrm{expand}(A,m)} & n=m+1
\end{cases}
\]

对于 \(A=(\#,(a+1)@(b+1))\)
\[
\mathrm{expand}(A,n)=
\begin{cases}
(\#,a@(b+1))+1 & n=0\\
(\#,a@(b+1),\mathrm{expand}(A,m)@b) & n=m+1
\end{cases}
\]

对于 \(A=(\#,(a+1)@b)\ (\neg\exists c\,(b=c+1))\)
\[
\mathrm{expand}(A,n)=(\#,a@b,1@\mathrm{expand}(b,n))
\]

对于 \(A=(\#,a@b)\ (\neg\exists c\,(a=c+1))\)
\[
\mathrm{expand}(A,n)=(\#,\mathrm{expand}(a,n)@b)
\]

## 1.3 compare 函数

对于 \(A=(a_1@i_1,a_2@i_2,\dots,a_n@i_n),B=(b_1@j_1,b_2@j_2,\dots,b_m@j_m)\)
\[
\mathrm{is\_equal}(A,B)=\begin{cases}
\mathrm{true} & a_1=b_1,\ i_1=j_1,\ n=m=1\\
\mathrm{false} & a_1\neq b_1,\ i_1\neq j_1\\
\mathrm{false} & n\neq m\\
\mathrm{false} & i_1\neq j_1\\
\mathrm{false} & a_1\neq b_1,\ i_1=j_1\\
\mathrm{is\_equal}((a_2@i_2,\dots,a_n@i_n),(b_2@j_2,\dots,b_m@j_m)) & n,m>1
\end{cases}
\]
\[
\mathrm{is\_greater}(A,B)=\begin{cases}
\mathrm{true} & i_1>j_1\\
\mathrm{true} & a_1> b_1,\ i_1=j_1\\
\mathrm{true} & n>m=1,\ a_1=b_1\\
\mathrm{false} & i_1\le j_1\\
\mathrm{false} & a_1\le b_1,\ i_1=j_1\\
\mathrm{false} & n=1\ge m,\ a_1=b_1\\
\mathrm{is\_greater}((a_2@i_2,\dots,a_n@i_n),(b_2@j_2,\dots,b_m@j_m)) & n,m>1
\end{cases}
\]
\[
\mathrm{compare}(A,B)=\begin{cases}
1 & \mathrm{is\_greater}(A,B)\\
0 & \mathrm{is\_equal}(A,B)\\
-1 & \lnot(\mathrm{is\_greater}(A,B)\land\mathrm{is\_equal}(A,B))
\end{cases}
\]

> 原文为简体中文，此处**逐字转录**，未改动任何符号 / transcribed verbatim.
> 字母约定：§1.3 用 \(i_k\) 表示第 \(k\) 个分量的**第二坐标**（即 \((a_k@i_k)\) 里的 `@i_k`，本库记作 `comps_[k].second`），
> 用 \(a_k/b_k\) 表示**第一坐标**（`comps_[k].first`）。与 §1.1 的 \(b_k\) 为同一量，仅记法不同。
> 比较是**字典序**：主键为第二坐标 \(i_k\)（most significant），次键为第一坐标 \(a_k\)；分量全相等时**较长者更大**（前缀规则）。见第 2、5 节实现与 C5 注记。

---

## 2. 库实现 / Library implementation

- `WeakVeblen` 持有单个 `Ordinal ord_`（即 \((a@b)\) 表达式树；展开后也可能
  成为文章情形产生的序数表达式，如 \((\#,a@0)+1\) 或有限数 \(n\)）。
- `expand(n)` 重写 `ord_` 并返回 `*this`（一个 `WeakVeblen` 对象）。
  `to_string()` 渲染 LaTeX。
- `operator[](n)`（即 \(A[n]\)）等价于 \(\mathrm{expand}(A,n)\)，**不**改写 `*this`
  （返回副本），故连续索引 \(A[1],A[2],\dots\) 安全。
- 文章里的 \(+1\)、\(\hat{}\)、\(\mathrm{expand}(b,n)\) 由 `Ordinal` 核心
  （`core/Ordinal.hpp`，遵循 `study/notations/Ordinal.md`）提供；6 个 case
  **逐字**实现，未加任何增补解释。

### 内部 6 例（与第 1 节一一对应）

| case | 条件 | \(\mathrm{expand}(A,n)\) |
|---|---|---|
| 1 | \(A=(0)\) | \(n\) |
| 2 | \(A=(0@b_1,\#)\) | \((\#)\) |
| 3 | \(A=(\#,(a+1)@0)\) | \(n=0\to(\#,a@0)+1\)；\(n=m+1\to(\#,a@0)^{\mathrm{expand}(A,m)}\) |
| 4 | \(A=(\#,(a+1)@(b+1))\) | \(n=0\to(\#,a@(b+1))+1\)；\(n=m+1\to(\#,a@(b+1),\mathrm{expand}(A,m)@b)\) |
| 5 | \(A=(\#,(a+1)@b),\ \neg\exists c\,(b=c+1)\) | \((\#,a@b,1@\mathrm{expand}(b,n))\) |
| 6 | \(A=(\#,a@b),\ \neg\exists c\,(a=c+1)\) | \((\#, \mathrm{expand}(a,n)@b)\) |

> **条件读法 / reading the guards.** 文章里的
> "\((\neg\exists c\,(b=c+1))\)" 表示 \(b\) **不是后继**（即极限或 0）；
> "\((\neg\exists c\,(a=c+1))\)" 表示 \(a\) **不是后继**。
> 代码以 `aLast.isSuccessor()` / `bLast.isZero()` / `bLast.isSuccessor()`
> 在**分量**上判定（分量须为闭序数，见 `spec/notations/ordinal.md` C3）。

---

## 3. 输出格式 / Output format (LaTeX)

`to_string()` / `print()` / `operator<<` 输出 **LaTeX**，如 `(2@3, 1@0)`；
\((\#,a@0)+1\) 渲染为 `(2@3, 0@0) + 1`。库**不**计算数值；
`expand` 返回记号自身对象，需要 LaTeX 时再调 `to_string()`。
复合分量（如后继 \((\#,a@0)+1\)）在 \((a@b)\) 列表里会以括号包裹，
避免 `@` 比 `+` 先结合：`((2@3, 0@1) + 1)@0`。

---

## 4. 字符串语法 / String syntax

- 输入 `string_to_it()` / `operator>>`：ASCII 元组 `(a1@b1, a2@b2, ...)`，
  空格可选，省略 `@b` 视为 \(b=0\)；空串 `( )` 视为 \((0@0)\)（case 1 的 \(A=(0)\)）。
- `to_string()` 输出 LaTeX \((a_1@b_1,\dots,a_n@b_n)\)。
- 各 \(a_i,b_i\) 由 `Ordinal::parse` 解析（整数、\(\omega\)、`+`、`*`、`^`、括号）。

---

## 5. 约定（文章沉默处的最小补充 / Conventions where the article is silent）

> 全部标为 **C1–C4**，均为**最小**补充，未改写文章既有符号。
> All flagged **C1–C4**, minimal additions only.

- **C1 — 统一序数值类型 / unified value type**：见
  `spec/notations/ordinal.md`。分量 \(a_i,b_i\) 用 `Ordinal` 表达式树表示。
- **C2 — \(\omega\) 的 FS / FS of \(\omega\)**：见 `spec/notations/ordinal.md`。
  \(\mathrm{expand}(\omega,n)=n\)（文章未定义，采用标准）。
- **C3 — 分量算术走 CNF / CNF for component arithmetic**：case 条件里的
  "\(a=c+1\)" 等判断在各分量的 Cantor 正规形上求值；分量须为闭序数，
  否则 `toCnf()` 抛 `domain_error`（文章未定义 WV 表达式的序数取值）。
- **C4 — 不继承 `OrdinalNotation` / does NOT derive from `OrdinalNotation`**：
  `WeakVeblen` **直承 `Notation`**，而非 `OrdinalNotation`。
  原因有二：(a) 它是**序数对 \((a@b)\)** 记号，不是自然数序列
  （如 PrSS）记号；(b) 文章**未给出标准型算法**，也未给出 `compare` 算法
  （仅陈述"在标准型下满足字典序比较"这一*性质*，未给*算法*）。
  按"照文章写"（不补文章所无）：`normalize` / `successor` 不实现
  （基类抛 `UnsupportedOperation`）。**但 `compare` 已有现成算法**——用户后续
  提供了 §1.3（`is_equal` / `is_greater` / `compare`），故 `Compare` 能力现已暴露（见 §2 实现与 C5 注记）。

- **C5 — §1.3 `is_greater` 的 `<=` 解读 / reading of the `<=` guards**：
  文章 `is_greater` 的兜底条件写作 `false 若 i_1 ≤ j_1` 与
  `false 若 a_1 ≤ b_1, i_1=j_1`。若按"自上而下首个命中即返回"的
  **字面**读法，递归分支（`n,m>1` 时的 `is_greater(tail)`）将**永不可达**——
  因为 `i_1 = j_1`（相等）会命中 `i_1 ≤ j_1` 而提前返回 `false`，
  永远走不到递归。文章本意显然是"*严格*小于才返回 false，相等则继续比较
  后续分量 / 递归"，故实现在这两个点取**严格 `<`**。这是**唯一**一处必要
  修正，且它**仅让文章自身的递归可达**——**未引入任何新行为**，
  属对意图的忠实解读（不增补）。序数算术落在 `core/Ordinal` 的 CNF
  比较（分量须为闭序数，见 `spec/notations/ordinal.md` C3；若某分量为 WV
  表达式则按 §1.3 递归比较该 WV 表达式）。

> ⚠ **残留开放项 / still-open.** `normalize` 与 `successor` 文章仍**未给**
> 算法，故不实现（基类抛 `UnsupportedOperation`）。`compare` 现已按 §1.3
> 实现（见 C5）。若日后要加标准型，须先确定标准型定义——超出文章文本，
> 需用户确认。
> `normalize` / `successor` are still NOT given by the article, so they stay
> unimplemented. `compare` is now implemented per §1.3 (see C5).

---

## 6. 接口映射 / API mapping

| 文章 / Article | 库调用 / Library call | 返回 / Returns |
|---|---|---|
| \(\mathrm{expand}(A,n)\)（第 n 基础项） | `expand(n)` 或 `operator[](n)` | `WeakVeblen&`（自身，已重写）/ 副本 |
| 渲染 / render | `to_string()` | LaTeX 字符串 |
| 解析 / parse | `string_to_it()` | — |

`reduce()`（反复 `expand(1)` 直到符号形式稳定）由基类提供；
文章无此概念，行为为"反复一次重写步"。

---

## 7. 测试向量 / Test vectors (golden)

见 `tests/unit/test_weakveblen.cpp`：6 个 case 各取独立对象（因 `expand` 改写
`*this`），输出与第 2 节表格一致，例如：

```
case1  (0@0)        expand(3) = 3
case2  (0@5, 1@3)  expand(1) = (1@3)
case3a (2@3, 1@0)  expand(0) = (2@3, 0@0) + 1
case3b (2@3, 1@0)  expand(1) = ((2@3, 0@0) ^ (2@3, 0@0) + 1)
case4a (2@3, 1@1)  expand(0) = (2@3, 0@1) + 1
case4b (2@3, 1@1)  expand(1) = (2@3, 0@1, ((2@3, 0@1) + 1)@0)
case5  (2@3, 1@ω)  expand(3) = (2@3, 0@ω, 1@3)
case6  (2@4, ω@3)   expand(3) = (2@4, 3@3)
```

`compare`（§1.3，主键第二坐标 `@b`、次键 `@a`、较长者更大）向量：

```
cmp((1@0),  (0@0))  =  1   # 次键 a: 1 > 0
cmp((0@1),  (0@0))  =  1   # 主键 i: 1 > 0
cmp((0@0),  (0@1))  = -1   # 镜像
cmp((1@0,1@0), (1@0))    =  1   # 前缀规则：较长者更大
cmp((1@0), (1@0,1@0))  = -1   # 镜像
cmp((1@0,0@0), (1@0,1@0)) = -1   # 分量1 次键 a: 0 < 1
cmp((2@3,1@0), (2@3,0@0)) =  1   # 分量1 次键 a: 1 > 0
cmp((0@5),  (100@0)) =  1   # 主键 i 主导 (5 > 0)
cmp((5@0),  (1@0))  =  1   # i 相等, 次键 a: 5 > 1
cmp((0@1),  (0@0))  = -cmp((0@0),(0@1))   # 反对称
```
