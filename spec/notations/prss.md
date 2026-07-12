# PrSS (Primitive Sequence System — 阶差型 / difference)

- **Family / 族**: ordinal (序数记号)
- **Subfamily / 子族**: sequence (自然数序列记号)
- **Style / 风格**: difference (阶差型)
- **Compare / 比较**: DEFINED — lexicographic on the sequence values (字典序)
- **Supported ops / 支持运算**: FromString, ToString, Expand, ExpandTo, Normalize, Compare, Successor
- **`normalize()` / `isSuccessor()`**: 非虚的 `OrdinalNotation` 基类（继承 `Notation`）函数（所有序数序列记号共用同一份实现，仅由 `baseVal_` 区分；不属求值类）
- **Evaluate / 求值**: **not provided / 不提供**（一律不求值 / never evaluated）

> PrSS occupies the `ordinal/sequence/difference/` slot in the taxonomy. It is the
> representative **difference-type (阶差型)** natural-number sequence notation.
> PrSS 对应 taxonomy 中的 `ordinal/sequence/difference/`（阶差型）槽。

## Output format (LaTeX) / 输出格式（LaTeX）

`to_string()` / `print()` / `operator<<` emit **LaTeX**, e.g.
`(0, 1, 2)`.
The library never computes a numeric/ordinal value. `expand(m)` /
`expand_to(M)` / `reduce()` **rewrite the internal sequence and return the
notation object itself** (a `PrSS`), never a string; call `to_string()` to
render LaTeX on demand. `string_to_it()` / `operator>>` accept **ASCII**
`(0, 1, 2)` input — they do NOT parse LaTeX.

`to_string()` / `print()` / `operator<<` 输出 **LaTeX**，例如 `(0, 1, 2)`。
库不计算数值/序数；`expand(m)` / `expand_to(M)` / `reduce()` **重写内部序列并返回
记号自身对象**（一个 `PrSS`），而非字符串；需要 LaTeX 时再调用 `to_string()`。
`string_to_it()` / `operator>>` 接受 **ASCII** 输入 `(0, 1, 2)`，不解析 LaTeX。

## Definition / 定义

A sequence \(A = (a_1, a_2, \dots, a_n)\) must satisfy / 序列 \(A\) 须满足：

1. \(a_i \in \mathbb{N}\).
2. \(a_{i+1} - a_i \le 1\) （相邻差不超过 1）.
3. \(a_1 = 0\).
4. \(a_{i+1} - a_i = 0 \Rightarrow a_{i+2} - a_{i+1} \le 0\)
   （平台之后不得再上升）.

Notation / 记法：the \(i\)-th element of \(A\) is \(a_i\) (written "`i th A = a_i`").

### Auxiliary / 辅助

- Concatenation / 拼接：
  \((a_1,\dots,a_n) \oplus (b_1,\dots,b_n) = (a_1,\dots,a_n, b_1,\dots,b_n)\).
- Folded concatenation / 折叠拼接：
  \(A \bigoplus_{i=a}^{n} f(i) = (A \bigoplus_{i=a}^{n-1} f(i)) \oplus f(n)\).

### Expansion / 展开

Empty / 末尾为 0（后继）/ General cases:

\[
\begin{aligned}
\text{expand}(A,m) &= () \\
\text{expandLen}(A,m) &= ()
\end{aligned}
\qquad (A = ())
\]

\[
\begin{aligned}
\text{expand}(A,m) &= (a_1, a_2, \dots, a_n) \\
\text{expandLen}(A,m) &= (a_1, a_2, \dots, a_n)
\end{aligned}
\qquad (A = (a_1,\dots,a_n,0))
\]

Otherwise (general \(A = (a_1,\dots,a_n)\)) / 其它情形（一般 \(A\)）：

- Let \(br = \max\{\,k \mid a_k < a_n\,\}\) — the **rightmost** column index
  whose value is strictly less than the last element \(a_n\)
  （从右到左第一个比 \(a_n\) 小的列标）.
- Let \(L = n - br\) — the length of the **tail** \((a_{br+1},\dots,a_n)\).

\[
\text{expandLen}(A,m) =
\begin{cases}
(a_1, a_2, \dots, a_n - 1) & m = 0 \\[4pt]
\text{expandLen}(A, k) \oplus \bigl((m+n-L)\ \text{th}\ \text{expandLen}(A,k)\bigr) & m = k+1
\end{cases}
\]

\[
\text{expand}(A,m) =
\begin{cases}
\text{expandLen}(A, 0) & m = 0 \\[4pt]
\text{expandLen}(A,\, m\cdot L - 1) & m > 0
\end{cases}
\]

> **Indexing note / 索引说明.** The element appended at stage \(m=k+1\)
> is the \((m+n-L)\)-th element of \(\text{expandLen}(A,k)\) — i.e. of the
> **running (already-grown) sequence**, *not* of the original \(A\). As the
> sequence is built up, later appends read from the tail that has accumulated
> so far. The article defines **no cyclic wrap**; none is applied. An index
> that falls beyond the running sequence is simply out of the article's literal
> domain (the code throws a defensive `std::out_of_range`).

## Expansion trace / 展开轨迹

For \(A = (0, 1, 2)\): \(n=3,\ a_n=2,\ br=2,\ L=1\).
\(m>0\) maps to \(\text{expand}(A,m)=\text{expandLen}(A, m\cdot L-1)
= \text{expandLen}(A, m-1)\), and each append reads from the running sequence:

```
expand((0,1,2), 0) = expandLen((0,1,2), 0)          = (0, 1, 1)
expand((0,1,2), 1) = expandLen((0,1,2), 0)          = (0, 1, 1)
expand((0,1,2), 2) = expandLen((0,1,2), 1)          = (0, 1, 1, 1)
expand((0,1,2), 3) = expandLen((0,1,2), 2)          = (0, 1, 1, 1, 1)
```

The telling case is \(A = (0, 1, 2, 2)\): \(n=4,\ a_n=2,\ br=2,\ L=2\),
\(m>0 \Rightarrow \text{expand}(A,m)=\text{expandLen}(A, 2m-1)\):

```
expand((0,1,2,2), 1) = expandLen((0,1,2,2), 1)     = (0, 1, 2, 1, 2)
expand((0,1,2,2), 2) = expandLen((0,1,2,2), 3)     = (0, 1, 2, 1, 2, 1, 2)
```

The library stops at the symbolic form; it never reduces
\((0,1,2,1,2,1,2)\) to a numeral / 库停在符号形式，绝不把 \((0,1,2,1,2,1,2)\) 坍缩成数。

## String syntax (parser) / 字符串语法

- Input via `string_to_it()` / `operator>>`: ASCII tuple / ASCII 元组
  `(0, 1, 2)` (whitespace optional / 空格可选).
- `to_string()` / `print()` / `operator<<` output LaTeX / 输出 LaTeX:
  `(0, 1, 2)`.

## API mapping / 接口映射

| Definition / 定义 | Library call / 库调用 | Returns / 返回 |
|---|---|---|
| `expand(A, m)` (m-th fundamental term) | `expand(m)` | `PrSS&` (self, rewritten) |
| `expandLen(A, M)` (append M tail copies) | `expand_to(M)` | `PrSS&` (self, rewritten) |
| render / 渲染 | `to_string()` | LaTeX string |
| parse / 解析 | `string_to_it()` | — |

`reduce()` repeatedly applies `expand(1)` until the symbolic form is stable.

## Standard form (normalize) / 标准型

PrSS supports `normalize()` (the **standard form / 标准型** computation): it
builds a canonical PrSS from the input's second element and expands it (using
PrSS's own `expand`) until it dominates the input; the result replaces the
sequence.

- Canonical starter / 规范起点: `[0, a_2]` for PrSS (the base element is 0).
  **⚠ to be verified / 待核对**.
- The loop is stall-guarded so it always terminates; on a stall or a
  non-standard input the sequence is left unchanged.
- ⚠ The exact PrSS standard-form semantics (and whether `normalize` should
  also enforce the four defining conditions) need confirmation.

## Comparison / 比较

`compare()` is **lexicographic (字典序)** on the sequence values. Ordinal
notations have a well-defined order; for a natural-number sequence notation
the order is the lexicographic order of the sequences / 序数记号有良定义序；对自然数序列记号，
比较即序列的字典序。 Cross-family comparisons (e.g. PrSS vs a large-number
notation) still throw `NotComparable`.
