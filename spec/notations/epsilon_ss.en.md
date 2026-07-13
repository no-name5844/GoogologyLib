# ε_pSS / ε_ωSS (merged module · same type)

> Both notations come from the **SAME source article**
> `study/notations/"epsilon_nSS & epsilon_omegaSS.md"` (§1 = ε_pSS, §2 = ε_ωSS).
> Per the project convention — **notations written in the same source file are the
> same type and belong in one module** — they are merged here.
> Code: `include/googology/notations/ordinal/sequence/difference/epsilon_ss/EpsilonSS.hpp`
> (contains both classes `EpspSS` and `EpsOmegaSS`, verbatim in one file).

## Common properties

- **Family**: `ordinal / sequence / difference` (ordinal · natural-number sequence · difference-type)
- **Reference**: `study/notations/"epsilon_nSS & epsilon_omegaSS.md"`
- Both inherit `OrdinalNotation`, share `baseVal_ = 1`, and share the
  `rightmostLess_` / `expandLen_` / `compare` / `normalize` algorithm.
- **The ONLY difference is the per-step addition**:
  - ε_pSS has a parameter `p` (a cap): case 4 caps the added amount at `p`.
  - ε_ωSS has no `p` and no case 4: the gap `q = a_n - a_br` is added
    **in full** (unbounded) — it is the `p → ω` limit of ε_pSS.

> **Notation convention (user-clarified).** Two distinct indexings — do **not** conflate them:
> - `a_i` (or "i-th A") = the **i-th element** of the sequence A itself (element access). In code: `seq_[i-1]`.
> - `A[n]` (square brackets) = the **n-th term of the fundamental sequence** of the ordinal A denotes. By convention this equals `expand(A, n)`. In code: `expand(n)` returns `A[n]`.
> Element access (`a_n`) and fundamental-term access (`A[n]`) are **different** operations.

### Interface mapping (both)
- `expand(m)` → the article's `expand(A, m)`; `expand_to(M)` → `expandLen(A, M)`.
- Both **rewrite the internal sequence and return the notation's OWN object** — the library **never evaluates to a number or ordinal**.
- `operator[]` is overloaded so `A[n]` returns the n-th term of the fundamental sequence (i.e. `expand(A, n)`); it returns a **copy** and does **not** mutate `*this`, so `A[1]`, `A[2]`, … are independent.
- `isSuccessor()` follows the article's successor clause: a sequence ending in 1 is a successor, and `expand`/`expandLen` strip that trailing 1.
- `normalize()` implements the project's **universal standard-form definition** (an expression is a legal expression iff obtainable from a *limit expression* by finitely many expansions + taking a prefix). The user specified the limit expression for both as **(1, n)**, so the canonical starter is `(1, a_2)`. A non-standard input is left unchanged.
- `compare()` is NOT in the article, but per the user's specification the notation satisfies **lexicographic ordinal comparison under standard form** (consistent with Prss); cross-type comparison throws `NotComparable`. This is a user-specified property, not an article claim.

## ε_pSS (ε_p Sequence System)

- **Creator**: `zahin` (ε_pSS is named by zahin; see `EpspSS::creator()` in code).
- **Version**: `1` (base version).

### Definition
ε_pSS is a *difference-type* natural-number **sequence** notation. Its limit
expressions are of the form `(1, n)` — i.e. every valid sequence begins with `1`.
For a sequence `A = (a_1, a_2, …, a_n)` the only required conditions are:
1. `a_i ∈ ℕ`
2. `a_1 = 1`

> Unlike **PrSS**, the extra PrSS constraints (`a_{i+1}-a_i ≤ 1` and the
> "plateau-then-non-rise" rule) are **relaxed** here — arbitrary jumps are allowed.
> ε_pSS is a *parameterized generalization* of PrSS; setting the cap appropriately
> recovers the PrSS behaviour on gap-1 sequences.

### Expansion
Auxiliary functions:
1. `(a_1,…,a_n) ⊕ (b_1,…,b_n) = (a_1,…,a_n, b_1,…,b_n)`
2. `A ⊕^n_{i=a} f(i) = (A ⊕^{n-1}_{i=a} f(i)) ⊕ f(n)`

`expand` function:
- Empty sequence `A=()`: `expand(A,m) = expandLen(A,m) = ()`.
- Ending in 1, `A=(a_1,…,a_n,1)`: `expand(A,m)=expandLen(A,m)=(a_1,…,a_n)` (strip trailing 1).
- Otherwise let `br = max{n−i | a_{n−i} < a_n}` (rightmost column whose value is strictly less than `a_n`, 1-based); `L = n−br`; `q = a_n − a_br` (since `a_br < a_n`, `q ≥ 1`).

```
expandLen(A,m)=
  (a_1,…,a_n−1)                                     m=0
  expandLen(A,k) ⊕ ((m+n−L) th expandLen(A,k))         m=k+1 ∧ a_n = a_k+1
  expandLen(A,k) ⊕ ((n+m−1) th expandLen(A,k) + q−1)  m=k+1 ∧ a_n = a_k+q (1<q≤p)
  expandLen(A,k) ⊕ ((n+m−1) th expandLen(A,k) + p)     m=k+1 ∧ a_n − a_k > p

expand(A,m)=
  expandLen(A,0)          m=0
  expandLen(A, m·L−1)     m>0
```

The two index forms are **kept exactly as written in the article**: case 2 uses
`(m+n−L) th expandLen(A,k)` (the gap-1 case), while cases 3/4 use
`(n+m−1) th expandLen(A,k) + q−1 / +p`. The indexed element is taken from
the **running** sequence `expandLen(A,k)` at stage `k=m−1`, *not* from the
original `A`. No reinterpretation or unification is applied.

### Implementation notes (C++ reference branch)
- The C++ class `googology::ordinal::EpspSS` implements **only** the formulas
  above, verbatim. The element appended at stage `m=k+1` is the `X`-th element
  of the **running** sequence `expandLen(A,k)` (with `X = m+n−L` or `n+m−1`
  per the case), **not** of the original `A`. In code this is `seq_` after the
  prior appends; the appended value is `seq[X-1] + add`.
- **No cyclic closure is added.** The article defines no wrap. When the article's
  index `X` lies beyond the running sequence (an input outside the article's
  literal domain), the code throws `std::out_of_range` — a defensive guard, not
  a reinterpretation of the formula.
- **`compare` is NOT defined by the article**, but per the user's specification
  the notation satisfies lexicographic ordinal comparison under standard form
  (consistent with Prss); cross-type comparison throws `NotComparable`. This is a
  user-specified property, not an article claim.
- **`isSuccessor()` follows the article's successor clause**; `normalize()` follows
  the project's universal standard-form definition + the user-specified limit
  expression `(1,n)`. Both are driven by the user's clarifications, not a personal
  opinion.

## ε_ωSS (ε_ω Sequence System)

### Definition
ε_ωSS is a *difference-type* natural-number **sequence** notation, the **ω-limit**
of ε_pSS (no cap on the added amount).
- Family as ε_pSS; **Parameter**: none (no `p`, no case 4).
- For `A = (a_1, …, a_n)`: `a_i ∈ ℕ`, `a_1 = 1`; the extra PrSS constraints
  are likewise relaxed.
- ε_ωSS = ε_pSS **without the parameter `p`** (and without its case 4): the gap
  `q = a_n - a_br` is added **in full**, unbounded — the `p → ω` limit.

### Expansion
Identical to ε_pSS but **without case 4** (no cap):

```
expandLen(A,m)=
  (a_1,…,a_n−1)                           m=0
  expandLen(A,k) ⊕ ((m+n−L) th expandLen(A,k))      m=k+1 ∧ a_n = a_k+1
  expandLen(A,k) ⊕ ((n+m−1) th expandLen(A,k) + q−1)  m=k+1 ∧ a_n = a_k+q
expand(A,m)= (same as ε_pSS)
```

Case 3 adds `q−1` in full (unbounded). The rest (running sequence, no closure
added, `compare`/`isSuccessor`/`normalize` conventions) is identical to ε_pSS.

### Implementation notes (C++ reference branch)
- The C++ class `googology::ordinal::EpsOmegaSS` implements the above verbatim
  (no `p`, no case 4). The rest is identical to the ε_pSS implementation notes.

## Multi-language
Per the project convention, both notations must eventually be ported to **C, Java,
Python, and Lean4** (each on its own git branch), all aligned to this spec and the
golden vectors on `master`. The C++ implementation is the current reference.
