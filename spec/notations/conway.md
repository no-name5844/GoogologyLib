# Conway chained-arrow notation (康威链式箭头表示法)

- **Family / 族**: number (large-number notation / 大数记号)
- **Subfamily / 子族**: —
- **Style / 风格**: —
- **Compare / 比较**: UNDEFINED (throws `NotComparable` / 抛 `NotComparable`)
- **Supported ops / 支持运算**: FromString, ToString, Expand, ExpandTo
- **Evaluate / 求值**: **not provided / 不提供**（一律不求值 / never evaluated）

## Output format (LaTeX) / 输出格式（LaTeX）

`to_string()` / `print()` / `operator<<` emit **LaTeX**, e.g. `3 \rightarrow 3 \rightarrow 2`.
The library never computes a numeric value. `expand(n)` / `expand_to(len)` /
`reduce()` **rewrite the internal chain and return the notation object itself**
(a `Conway`), never a string; call `to_string()` to render LaTeX on demand.
`string_to_it()` / `operator>>` accept **ASCII** (`3 -> 3 -> 2`) or **Unicode**
(`3 → 3 → 2`) input — they do NOT parse LaTeX.

`to_string()` / `print()` / `operator<<` 输出 **LaTeX**，例如 `3 \rightarrow 3 \rightarrow 2`。
库不计算数值；`expand(n)` / `expand_to(len)` / `reduce()` **重写内部链并返回记号自身对象**
（一个 `Conway`），而不是字符串；需要 LaTeX 时再调用 `to_string()`。
`string_to_it()` / `operator>>` 接受 **ASCII**（`3 -> 3 -> 2`）或 **Unicode**
（`3 → 3 → 2`）输入，不解析 LaTeX。

## Definition / 定义

链 \(x_0 \rightarrow x_1 \rightarrow \dots \rightarrow x_k\) 遵循 / A chain obeys:

1. \(a \rightarrow b = a^b\)  （长度 2 的基例 / base, length 2）
2. \(X \rightarrow 1 \rightarrow Y = X\)
3. \(X \rightarrow a \rightarrow b = X \rightarrow (X \rightarrow a-1 \rightarrow b) \rightarrow b-1\)

（约定 / Convention: 以尾随 `1` 结束的链，即 \(X \rightarrow a \rightarrow 1\)，化简为 \(X \rightarrow a\)，
与规则 2 及基例一致。 / a chain ending in a trailing `1`, i.e. \(X \rightarrow a \rightarrow 1\), reduces to
\(X \rightarrow a\), consistent with rule 2 and the base case.）

## String syntax (parser) / 字符串语法

- ASCII `->` 或 Unicode `→`，各项以箭头分隔 / ASCII `->` or Unicode `→`, entries separated by arrows.
- 例 / Examples: `3 -> 2`, `4 -> 3`, `3 -> 3 -> 2`.

## Expansion (how to compute) / 展开（如何计算）

本库**不计算数值**；`expand` 提供按规则 3 的一步重写，`reduce` 反复应用直到符号最简。
The library **does not compute values**; `expand` gives one rewrite per rule 3, and `reduce`
repeats it until the simplest symbolic form.

轨迹示例 / Trace example:

```
3 -> 3 -> 2
 → 3 -> (3 -> 2 -> 2) -> 1        (rule 3)
 → 3 -> (3 -> 2 -> 2)            (rule 2, drop trailing 1)
 → 3 -> (3 -> (3 -> 1 -> 2) -> 1)  (rule 3 on inner)
 → 3 -> (3 -> (3 -> 1))          (rule 2 on inner)
 → 3 -> (3 -> 3)                (3 -> 1 base = 3)
 → 3 -> 3^3                     (3 -> 3 base = 3^3)
```

（最终符号形式为 `3 \rightarrow (3^{3})`（即 `3^(3^3)`）；不计算其终值。）
(The final symbolic form is `3 \rightarrow (3^{3})` = `3^(3^3)`; its value is NOT computed.)

## Comparison / 比较

不提供。大数记号比较通常未定义；调用 `compare()` 抛 `NotComparable`。
Not provided. Large-number comparison is generally undefined; calling `compare()` throws `NotComparable`.
