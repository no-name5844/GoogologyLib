# ε_ωSS (epsilon-omega Sequence System)

**Family / 族**: `ordinal / sequence / difference` (序数 · 自然数序列 · 阶差型)
**Parameter / 参数**: none — this is the **ω-limit** of ε_pSS (no cap on the
added amount).
**Reference / 参考**: `study/notations/epsilon_nSS & epsilon_omegaSS.md` (section 2).

---

## 1. Definition / 定义

ε_ωSS is a natural-number **sequence** notation of the *difference* (阶差型) kind.
For a sequence $A = (a_1, a_2, \dots, a_n)$:

1. $a_i \in \mathbb{N}$
2. $a_1 = 0$

As with ε_pSS, the extra PrSS constraints are **relaxed** (arbitrary jumps allowed).
ε_ωSS is ε_pSS **without the parameter `p`** (and without its case 4): the gap
$q = a_n - a_{br}$ is added **in full**, unbounded. It is the $p \to \omega$ limit
of ε_pSS.

For an element $a_i$: column index (列标) $= i$, and $i\ \text{th}\ A = a_i$.

---

## 2. Expansion / 展开

### 2.1 Auxiliary functions / 辅助函数
1. $(a_1,\dots,a_n) \oplus (b_1,\dots,b_n) = (a_1,\dots,a_n, b_1,\dots,b_n)$
2. $A \bigoplus^n_{i=a} f(i) = \big(A \bigoplus^{n-1}_{i=a} f(i)\big) \oplus f(n)$

### 2.2 `expand` function / expand 函数

Empty sequence $A=()$: $expand(A,m)=expandLen(A,m)=()$.

Sequence ending in $1$, $A=(a_1,\dots,a_n,1)$:
$expand(A,m)=expandLen(A,m)=(a_1,\dots,a_n)$ (strip the trailing $1$).

Otherwise, define $br$, $L=n-br$, and $q=a_n-a_{br}$ exactly as for ε_pSS. Then:
$$
expandLen(A,m)=
\begin{cases}
(a_1,a_2,\dots,a_n-1) & m=0 \\[4pt]
expandLen(A,k)\;\oplus\;\big((m+n-L)\ \text{th}\ A\big)       & m=k+1 \ \land\ a_n = a_k+1 \\[4pt]
expandLen(A,k)\;\oplus\;\big((n+m-1)\ \text{th}\ A + q\big) & m=k+1 \ \land\ a_n = a_k+q
\end{cases}
$$

$$
expand(A,m)=
\begin{cases}
expandLen(A,0)              & m=0 \\[4pt]
expandLen(A,\, m\cdot L - 1) & m>0
\end{cases}
$$

The two index forms are **kept exactly as written in the article**:
case 2 uses $(m+n-L)\ \text{th}\ A$ (the gap-1 case), and
case 3 uses $(n+m-1)\ \text{th}\ A + q$ with $q$ added in full.
There is no case 4 (no cap). No reinterpretation or unification is applied.

---

## 3. Interface mapping / 接口映射
- `expand(m)` → `expand(A, m)`; `expand_to(M)` → `expandLen(A, M)`.
- Both rewrite the internal sequence and return the notation's OWN object
  (never evaluated to a value).

---

## 4. Implementation notes / 实现说明 (C++ reference branch)
- The C++ class `googology::ordinal::EpsOmegaSS` implements **only** the
  formulas above, verbatim. `expandLen` accesses the *original* $A$'s
  $X$-th element literally as written.
- **No cyclic closure is added.** As with ε_pSS, an out-of-range article
  index throws `std::out_of_range` (a defensive guard, not a
  reinterpretation). Such inputs are outside the article's literal domain.
- **`compare` and `normalize` are NOT defined by the article**, so they are
  left to the base class and throw (`NotComparable` / `UnsupportedOperation`).
- Per the project convention this notation must eventually be ported to
  **C, Java, Python, and Lean4** (each on its own git branch), aligned to
  this spec and the golden vectors on `master`.

---

## 5. Multi-language / 多语言
Must be ported to **C, Java, Python, Lean4** (own branches), aligned to this spec
and the golden vectors on `master`. The C++ implementation is the current reference.
