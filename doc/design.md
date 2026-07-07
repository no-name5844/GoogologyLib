# GoogologyLib — Design (C++ branch)

This document is the contract for the whole library. It is language-neutral in
spirit; the C++ branch is the reference implementation. Other language branches
(C / java / python / Lean4) mirror the same logical skeleton.

## 1. 序数记号 ≠ 大数记号 (the key distinction)

These are two fundamentally different kinds of object and must not be confused:

| | 序数记号 (ordinal) | 大数记号 (large-number) |
|---|---|---|
| represents | ordinals (well-ordered) | integers / functions |
| `compare` | **well-defined** (ordinal order) | **generally UNDEFINED (不明确)** |
| `expand` | fundamental sequence α[n] | one rewrite / array step |
| example | Cantor, Veblen, Buchholz, Y, PPS | Knuth ↑, Conway →, BEAF, Hyper-E |

Consequences baked into the architecture:

- The generic base `Notation` provides `compare()`, but for **large-number
  notations it throws `NotComparable`** and `comparable()` returns `false`.
  Comparison across families is never defined.
- A notation "includes other operations" (其他运算也要包含进去) by **declaring
  them in its interface** via the capability bitset, even if it does not
  implement all of them. Unsupported ops throw `UnsupportedOperation`.

## 2. Taxonomy (hierarchical)

```
notations/
├─ ordinal/                         (compare is defined)
│   ├─ sequence/                    (自然数序列记号: Y, PPS live here)
│   │   ├─ difference/              (阶差型)   -> pps
│   │   └─ marked_parent/          (标记父向型) -> y_sequence
│   │   (other ordinal sequence notations may be filed here later)
│   └─ cnf/ veblen/ buchholz/ taranovsky/ bocf/ mountain/   (classic ordinal)
├─ number/                         (compare UNDEFINED)
│   ├─ knuth/     (高德纳箭头)      <- implemented
│   ├─ conway/    (康威链式箭头)    <- implemented
│   └─ beaf/ hyper_e/ extensible_e/ bird/   (future slots)
└─ hierarchy/                      (bridge ordinal -> number)
    └─ fgh/ hardy/ shg/            (future slots)
```

Every notation exposes `name()`, `family()`, `subfamily()`, `style()` so the
registry can answer queries like "list all ordinal/sequence/marked_parent
notations" without relying on directory layout.

## 3. Capability model

```cpp
enum class Op { Parse, Serialize, Normalize, Compare, Expand, ExpandTo, Evaluate, Successor };
class Capabilities { void set(Op); bool has(Op) const; };
```

Each notation overrides `capabilities()`. The base supplies default
implementations that throw, so a partial notation is still a valid `Notation`
and the interface is always complete.

## 4. Operation semantics (per family)

- **Parse / Serialize**: string <-> internal form.
- **Normalize**: canonical / standard form (ordinal: standard form; number: simplification).
- **Compare**: ordinal -> ordinal order; number -> **throws NotComparable**.
- **Expand(n)**: ordinal -> fundamental sequence nth term α[n]; number -> one
  rewrite step, applied n times.
- **ExpandTo(len)**: ordinal -> expand to target length; number -> expand to
  target size (both available; see user decision: "两者都要").
- **Evaluate**: number -> compute the value (BigInt; may overflow for huge
  inputs under the default int64_t — switch to cpp_int for real use).
- **Successor**: mainly ordinal.

## 5. Status of this C++ branch

Implemented (real, auditable):
- `number/knuth` — Knuth up-arrow (高德纳箭头)
- `number/conway` — Conway chained arrow (康威链式箭头)

Both deliberately do **not** support `Compare` (large-number comparison is
undefined). All other notations above are future slots.

## 6. Integer type

`BigInt` defaults to `int64_t` (no dependency, builds anywhere). Define
`GOOGOLOGY_USE_BOOST` for `boost::multiprecision::cpp_int` when evaluating
values that exceed 2^63.

## 7. Multi-branch layout

`master` holds language-neutral `spec/` and `tests/vectors/` (golden test
vectors). Each language branch implements the same logical skeleton. See the
top-level README.
