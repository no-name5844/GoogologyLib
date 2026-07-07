# Conway chained-arrow notation (康威链式箭头表示法)

- **Family**: number (large-number notation)
- **Subfamily**: —
- **Style**: —
- **Compare**: UNDEFINED (throws `NotComparable`)
- **Supported ops**: Parse, Serialize, Expand, Evaluate

## Definition

A chain \(x_0 \rightarrow x_1 \rightarrow \dots \rightarrow x_k\) obeys:

1. \(a \rightarrow b = a^b\)  (base, length 2)
2. \(X \rightarrow 1 \rightarrow Y = X\)
3. \(X \rightarrow a \rightarrow b = X \rightarrow (X \rightarrow a-1 \rightarrow b) \rightarrow b-1\)

(Convention used here: a chain ending in a trailing `1`, i.e. \(X \rightarrow a \rightarrow 1\),
reduces to \(X \rightarrow a\); this is consistent with rule 2 and the base case.)

## Representation in code

A chain is stored as `std::vector<BigInt>` `[x0, x1, ..., xk]`. `evaluate()`
applies the rules recursively. `expand(n)` applies one rewrite step (rule 3)
`n` times; the inner sub-chain is collapsed to its value so the result stays a
flat chain (a fully symbolic nested form is a future extension).

## String syntax (parser)

- ASCII `->` or Unicode `→`, entries separated by arrows.
- Examples: `3 -> 2` (9), `4 -> 3` (64), `3 -> 3 -> 2` (3^27 = 7625597484987).

## Example (evaluate)

| expression | value |
|---|---|
| `3 -> 2` | 9 |
| `4 -> 3` | 64 |
| `3 -> 3 -> 2` | 3^27 = 7625597484987 |

## Comparison

Not provided. Large-number comparison is generally undefined; calling
`compare()` throws `NotComparable`.
