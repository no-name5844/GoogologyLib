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
expandLen(A,k)\;\oplus\;\big((n+m-1)\ \text{th}\ A\big)            & m=k+1 \ \land\ a_n = a_k+1 \\[4pt]
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

**Two notes on the published formula** (resolved during implementation):

- **The two index forms coincide.** The original text writes the case-2 position as
  $(m+n-L)\ \text{th}\ A$ but cases 3/4 as $(n+m-1)\ \text{th}\ A$. These are
  *identical* in every valid input: case 2 is the gap-$1$ case ($a_n = a_k+1$),
  which forces $a_{n-1}=a_n-1<a_n$, hence $br = n-1$ and $L=1$; then
  $(m+n-L) = (m+n-1) = (n+m-1)$. We therefore use the **unified** index
  $(n+m-1)\ \text{th}\ A$ for all cases and only vary the added amount
  ($0$ for case 2, $q$ for case 3, $p$ for case 4).
- **Closure / 封闭 (⚠ implementation note).** The index $(n+m-1)\ \text{th}\ A$ lies
  beyond the sequence once $m > L$. We wrap it **cyclically within the tail**
  $[br+1,\dots,n]$: the $m$-th appended element reads
  $a_{\,br + ((m-1)\bmod L) + 1\,}$ (for cases 3/4, with $+q$ or $+p$).
  This keeps the algorithm **total** (always defined) without changing the
  published values for $m \le L$.

### Interface mapping / 接口映射
- `expand(m)` → the definition's `expand(A, m)`.
- `expand_to(M)` → `expandLen(A, M)` (length-parameterized expansion).
- Both **rewrite the internal sequence and return the notation's OWN object** —
  the library **never evaluates to a number or ordinal** (一律不求值).

---

## 3. Comparison / 比较
`compare()` is **lexicographic** (字典序) on the sequence values — exactly the
order used by `YSequence::_compare` in the reference `AutoGuogaoMachine/YSequence.cpp`.
Ordinal notations have a well-defined order, so this is defined in principle.
Cross-family comparisons (e.g. ε_pSS vs a large-number notation) still throw `NotComparable`.

---

## 4. Standard form (normalize) / 标准型
`normalize()` computes the **standard form** via the same algorithm as
`YSequence::checkStandardAndNonMaximum`, adapted to ε_pSS's own (parameterized)
expansion. ⚠ The **canonical starter** $[0, a_2]$ (vs YSequence's $[1, a_2+1]$,
because ε_pSS's base element is $0$) and the exact ε_pSS standard-form semantics
need verification. The loop is **stall-guarded** so it always terminates; on a stall
or a non-standard input the sequence is left unchanged.

---

## 5. Multi-language / 多语言
Per the project convention, this notation must eventually be ported to **C, Java,
Python, and Lean4** (each on its own git branch), all aligned to this spec and the
golden vectors on `master`. The C++ implementation is the current reference.
