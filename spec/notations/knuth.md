# Knuth up-arrow notation (高德纳箭头)

- **Family**: number (large-number notation)
- **Subfamily**: —
- **Style**: —
- **Compare**: UNDEFINED (throws `NotComparable`)
- **Supported ops**: Parse, Serialize, Expand, Evaluate

## Definition

\[
a \uparrow^c b =
\begin{cases}
a \uparrow b & c = 1 \\
a & b = 1 \\
a \uparrow^{c-1} (a \uparrow^c (b-1)) & c > 1,\; b > 1
\end{cases}
\]

where \(a \uparrow b = a^b\).

### Notes
- \(c=1\): ordinary exponentiation \(a^b\).
- \(c=2\): tetration \(a \uparrow\uparrow b\).
- \(c=3\): pentation, etc.
- The case \(b=1\) returns \(a\) for every height \(c\ge 1\).

## Representation in code

`a ^c b` is stored as an AST node `Arrow(a, c, Value(b))`. Expansion rewrites
one step: `a ^c b` → `a ^(c-1) (a ^c (b-1))` (for \(c>1, b>1\)).

## String syntax (parser)

- Height = **number of consecutive `^` characters**:
  - `^`  → c = 1 (ordinary exponentiation)
  - `^^` → c = 2 (tetration)
  - `^^^` → c = 3 (pentation), etc.
- The Unicode arrow `↑` is also accepted and normalized to `^`.

Examples: `2 ^ 3` (8), `2 ^^ 3` (16), `3 ^^ 2` (27).

## Example (evaluate)

| expression | value |
|---|---|
| `2 ^ 3` | 8 |
| `2 ^^ 3` | 16 |
| `3 ^^ 2` | 27 |

## Comparison

Not provided. Large-number comparison is generally undefined; calling
`compare()` throws `NotComparable`.
