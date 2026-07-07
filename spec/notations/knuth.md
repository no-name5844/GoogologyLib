# Knuth up-arrow notation (高德纳箭头)

- **Family / 族**: number (large-number notation / 大数记号)
- **Subfamily / 子族**: —
- **Style / 风格**: —
- **Compare / 比较**: UNDEFINED (throws `NotComparable` / 抛 `NotComparable`)
- **Supported ops / 支持运算**: FromString, ToString, Expand, ExpandTo
- **Evaluate / 求值**: **not provided / 不提供**（一律不求值 / never evaluated）

## Output format (LaTeX) / 输出格式（LaTeX）

`to_string()` / `print()` / `operator<<` emit **LaTeX**, e.g. `2 \uparrow\uparrow 3`.
The library never computes a numeric value; `expand(n)` / `reduce()` return the
symbolic rewrite, also in LaTeX (rewrite steps joined with `\to`).
`string_to_it()` / `operator>>` accept **ASCII** (`2 ^^ 3`) or **Unicode** (`2 ↑↑ 3`)
input — they do NOT parse LaTeX.

`to_string()` 输出 **LaTeX**，例如 `2 \uparrow\uparrow 3`。库不计算数值；
`expand(n)` / `reduce()` 返回符号化重写（亦为 LaTeX，重写步用 `\to` 连接）。
`string_to_it()` / `operator>>` 接受 **ASCII**（`2 ^^ 3`）或 **Unicode**（`2 ↑↑ 3`）
输入，不解析 LaTeX。

## Definition / 定义

\[
a \uparrow^c b =
\begin{cases}
a \uparrow b & c = 1 \\
a & b = 1 \\
a \uparrow^{c-1} (a \uparrow^c (b-1)) & c > 1,\; b > 1
\end{cases}
\]

其中 \(a \uparrow b = a^b\)。
where \(a \uparrow b = a^b\).

### Notes / 注记
- \(c=1\): 普通乘方 \(a^b\) / ordinary exponentiation.
- \(c=2\): 迭代幂次（tetration）\(a \uparrow\uparrow b\).
- \(c=3\): pentation，依此类推。
- \(b=1\) 时对任意高度 \(c\ge 1\) 返回 \(a\)。

## String syntax (parser) / 字符串语法

- Height = **连续 `^` 的个数 / number of consecutive `^` characters**:
  - `^`  → c = 1（普通乘方）
  - `^^` → c = 2（tetration）
  - `^^^` → c = 3（pentation），依此类推。
- Unicode 箭头 `↑` 也被接受并归一化为 `^` / the Unicode arrow `↑` is also accepted and normalized to `^`.

## Expansion (how to compute) / 展开（如何计算）

本库**不计算数值**；`expand` 提供按定义的一步重写，`reduce` 反复应用直到符号最简。
The library **does not compute values**; `expand` gives one rewrite per the definition, and
`reduce` repeats it until the simplest symbolic form.

单步规则 / One-step rule: for \(c>1, b>1\),
`a ↑^c b` → `a ↑^(c-1) (a ↑^c (b-1))`.

轨迹示例 / Trace example:

```
2 ↑↑ 3
 → 2 ↑ (2 ↑↑ 2)
 → 2 ↑ (2 ↑ 2)
```

（到此停止；不计算 `2 ↑ (2 ↑ 2)` 的终值 `16`。）
(Stop here; the final value `16` of `2 ↑ (2 ↑ 2)` is NOT computed.)

## Comparison / 比较

不提供。大数记号比较通常未定义；调用 `compare()` 抛 `NotComparable`。
Not provided. Large-number comparison is generally undefined; calling `compare()` throws `NotComparable`.
