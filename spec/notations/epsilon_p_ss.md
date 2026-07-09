# ε_pSS (epsilon-p Sequence System)

**Family / 族**: `ordinal / sequence / difference` (序数 · 自然数序列 · 阶差型)
**Parameter / 参数**: a natural number `p ∈ ℕ` (caps the amount added per expansion step).
**Reference / 参考**: `study/notations/epsilon_nSS & epsilon_omegaSS.md` (section 1).

---

## 1. Definition / 定义

ε_pSS is a natural-number **sequence** notation of the *difference* (阶差型) kind.
For a sequence $A = (a_1, a_2, \dots, a_n)$ the only required conditions are:

1. $a_i \in \mathbb{N}$
2. $a_1 = 0$

> Unlike **PrSS**, the extra PrSS constraints ($a_{i+1}-a_i \le 1$ and the
> "plateau-then-non-rise" rule) are **relaxed** here — arbitrary jumps are allowed.
> ε_pSS is a *parameterized generalization* of PrSS; setting the cap appropriately
> recovers the PrSS behaviour on gap-1 sequences.

For an element $a_i$ we write its **column index** (列标) as $i$, and $i\ \text{th}\ A = a_i$.

---

## 2. Expansion / 展开

### 2.1 Auxiliary functions / 辅助函数
1. $(a_1,\dots,a_n) \oplus (b_1,\dots,b_n) = (a_1,\dots,a_n, b_1,\dots,b_n)$
2. $A \bigoplus^n_{i=a} f(i) = \big(A \bigoplus^{n-1}_{i=a} f(i)\big) \oplus f(n)$

### 2.2 `expand` function / expand 函数

For the empty sequence $A = ()$:
$$expand(A,m)=(),\qquad expandLen(A,m)=()$$

For a sequence ending in $1$, $A = (a_1,\dots,a_n,1)$:
$$expand(A,m) = (a_1,\dots,a_n),\qquad expandLen(A,m) = (a_1,\dots,a_n)$$
(i.e. strip the trailing $1$.)

Otherwise, let
- $br = \max\{\,n-i \mid a_{n-i} < a_n\,\}$ — the **rightmost** column whose value is
  strictly less than $a_n$ (the last element); in 1-based indexing.
- $L = n - br$ — the length of the *tail* (columns $br+1,\dots,n$).
- $q = a_n - a_{br}$ — the gap between $a_n$ and that rightmost-smaller element.
  (Since $a_{br} < a_n$, we always have $q \ge 1$.)

Then:
$$
expandLen(A,m)=
\begin{cases}
(a_1,a_2,\dots,a_n-1) & m=0 \\[4pt]
expandLen(A,k)\;\oplus\;\big((m+n-L)\ \text{th}\ A\big)            & m=k+1 \ \land\ a_n = a_k+1 \\[4pt]
expandLen(A,k)\;\oplus\;\big((n+m-1)\ \text{th}\ A + q\big)  & m=k+1 \ \land\ a_n = a_k+q\ (1<q\le p) \\[4pt]
expandLen(A,k)\;\oplus\;\big((n+m-1)\ \text{th}\ A + p\big)  & m=k+1 \ \land\ a_n - a_k > p
\end{cases}
$$

$$
expand(A,m)=
\begin{cases}
expandLen(A,0)        & m=0 \\[4pt]
expandLen(A,\, m\cdot L - 1) & m>0
\end{cases}
$$

The two index forms are **kept exactly as written in the article**:
case 2 uses $(m+n-L)\ \text{th}\ A$ (the gap-1 case), while
cases 3/4 use $(n+m-1)\ \text{th}\ A + q$ / $+p$. No reinterpretation
or unification is applied.

---

## 3. Interface mapping / 接口映射
- `expand(m)` → the definition's `expand(A, m)`.
- `expand_to(M)` → `expandLen(A, M)` (length-parameterized expansion).
- Both **rewrite the internal sequence and return the notation's OWN object** —
  the library **never evaluates to a number or ordinal** (一律不求值).

---

## 4. Implementation notes / 实现说明 (C++ reference branch)
- The C++ class `googology::ordinal::EpspSS` implements **only** the
  formulas above, verbatim. `expandLen` accesses the *original* $A$'s
  $X$-th element literally as written (`X th A` = `seq[X-1]`).
- **No cyclic closure is added.** For some inputs the article's index
  $(n+m-1)\ \text{th}\ A$ lies beyond the original sequence; rather than
  silently wrapping, the code throws `std::out_of_range` (a defensive
  guard, not a reinterpretation of the formula). Such inputs are simply
  outside the article's literal domain.
- **`compare` and `normalize` are NOT defined by the article**, so they are
  left to the base class and throw (`NotComparable` / `UnsupportedOperation`).
  They are intentionally absent — do not add them as "opinions".
- Per the project convention this notation must eventually be ported to
  **C, Java, Python, and Lean4** (each on its own git branch), all
  aligned to this spec and the golden vectors on `master`.

---

## 5. Multi-language / 多语言
Per the project convention, this notation must eventually be ported to **C, Java,
Python, and Lean4** (each on its own git branch), all aligned to this spec and the
golden vectors on `master`. The C++ implementation is the current reference.
