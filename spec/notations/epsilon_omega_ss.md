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
2. $a_1 = 1$

As with PrSS, the extra PrSS constraints are **relaxed** (arbitrary jumps allowed).
ε_ωSS is ε_pSS **without the parameter `p`** (and without its case 4): the gap
$q = a_n - a_{br}$ is added **in full**, unbounded. It is the $p \to \omega$ limit
of ε_pSS.

For an element $a_i$: column index (列标) $= i$, and $i\ \text{th}\ A = a_i$.

> **Notation / 记号约定 (user-clarified).** Two distinct indexings — do **not** conflate them:
> - $a_i$ (or "$i\ \text{th}\ A$") = the **$i$-th element** of the sequence $A$ itself (element access). In code: `seq_[i-1]`.
> - $A[n]$ (square brackets) = the **$n$-th term of the fundamental sequence** of the ordinal $A$ denotes. By convention this equals $\text{expand}(A, n)$. In code: `expand(n)` returns $A[n]$.
> The article expresses the latter via the `expand(A,m)` function; some texts shorten it to $A[n]$. Element access ($a_n$) and fundamental-term access ($A[n]$) are **different** operations.

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
expandLen(A,k)\;\oplus\;\big((m+n-L)\ \text{th}\ \text{expandLen}(A,k)\big)       & m=k+1 \ \land\ a_n = a_k+1 \\[4pt]
expandLen(A,k)\;\oplus\;\big((n+m-1)\ \text{th}\ \text{expandLen}(A,k) + q-1\big) & m=k+1 \ \land\ a_n = a_k+q
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
case 2 uses $(m+n-L)\ \text{th}\ \text{expandLen}(A,k)$ (the gap-1 case),
and case 3 uses $(n+m-1)\ \text{th}\ \text{expandLen}(A,k) + q-1$ with $q-1$
added in full. The indexed element is taken from the **running** sequence
$\text{expandLen}(A,k)$ at stage $k=m-1$, *not* from the original $A$.
There is no case 4 (no cap). No reinterpretation or unification is applied.

---

## 3. Interface mapping / 接口映射
- `expand(m)` → `expand(A, m)`; `expand_to(M)` → `expandLen(A, M)`.
- Both rewrite the internal sequence and return the notation's OWN object
  (never evaluated to a value).

---

## 4. Implementation notes / 实现说明 (C++ reference branch)
- The C++ class `googology::ordinal::EpsOmegaSS` implements **only** the
  formulas above, verbatim. The element appended at stage $m=k+1$ is the
  $X$-th element of the **running** sequence $\text{expandLen}(A,k)$
  (with $X = m+n-L$ or $n+m-1$ per the case), **not** of the original
  $A$. In code this is `seq_` after the prior appends; the appended value
  is `seq[X-1] + add`.
- **No cyclic closure is added.** The article defines no wrap. When the
  article's index $X$ lies beyond the running sequence (an input outside
  the article's literal domain), the code throws `std::out_of_range` — a
  defensive guard, not a reinterpretation of the formula. Such inputs are
  outside the article's literal domain.
- **`compare` is NOT defined by the article**, but per the user's
  specification the notation satisfies **lexicographic ordinal comparison
  under standard form** (consistent with Prss). It is implemented as
  lexicographic order over the sequence — valid when both operands are
  in standard form; cross-type comparison throws `NotComparable`. This
  is a user-specified property, not an article claim.
- **`isSuccessor()` follows the article's successor clause**: a sequence
  ending in 1 is a successor, and `expand`/`expandLen` strip that
  trailing 1 (see the `<!-- A is Successor -->` clause above).
- **`normalize()` implements the project's UNIVERSAL standard-form definition**
  (an expression is a *legal expression* iff obtainable from a *limit
  expression* by finitely many expansions + taking a prefix). The user has
  specified the limit expression for ε_ωSS as **(1, n)**, so the canonical
  starter is `(1, a_2)`. A non-standard input is left unchanged. This is
  driven by the user's own clarifications (the universal definition + the
  marked limit expression), **not** by the article's ε_ωSS text — which does
  not define standard form — so it is faithful, not a personal opinion.
- **`operator[]` is overloaded** so that `A[n]` returns the n-th term of
  the fundamental sequence of the ordinal $A$ denotes — i.e. `expand(A, n)`
  (see the 记号约定 note above). It returns a copy and does **not** mutate
  `*this`, so `A[1]`, `A[2]`, … are independent.
- Per the project convention this notation must eventually be ported to
  **C, Java, Python, and Lean4** (each on its own git branch), aligned to
  this spec and the golden vectors on `master`.

---

## 5. Multi-language / 多语言
Must be ported to **C, Java, Python, Lean4** (own branches), aligned to this spec
and the golden vectors on `master`. The C++ implementation is the current reference.
