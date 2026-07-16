# Ns (Nonlinear successo · nonlinear successor)

> **Source / reference article**: `study/notations/Ns.md`
> **Creator**: the article is signed `by1:送到本到(UTF-8) 2:nnc` — two creators: `送到本到` (with an `(UTF-8)` note, likely an encoding remark) and `nnc`. `creator()` is proposed as `"送到本到, nnc"` (multiple authors joined by comma); **exact format to be confirmed by the user in the planning template** (design.md §3c).
> **Version**: the article gives no explicit version number (TBD).
> **Reference**: the same source article defines the auxiliary function $f$, the set $\mathbb{NS}$, $\psi(x)$, and the parameterized family `n,m-Ns` (Ns is its $(n,m)=(2,2)$ special case).

## 1. Definition

### 1.1 Auxiliary function $f$

$$ f(\alpha) = \begin{cases}
1 & \alpha = 0 \\
f(\beta)\cdot 2 & \alpha = \beta + 1 \\
f(expand(\alpha,2)) & \lnot\exists\beta\;(\alpha = \beta + 1)\quad(\alpha\text{ is a limit})
\end{cases} \qquad (\alpha\in On) $$

> Note: `expand(α,2)` is the 2nd term $\alpha[2]$ of $\alpha$'s fundamental sequence.
> **This function depends on another ordinal notation's fundamental-sequence definition** (see §2 and design.md §11).

### 1.2 General "$\alpha\,th\,A$"

For any set $A$ of ordinals with $|\!A\!|\ge\aleph_0$:
1. $1st, A = \min A$
2. $\alpha\,th\,A = \min\{x \mid x > \max\{\beta\,th\,A \mid \beta < \alpha\}\;\land\; x\in A\}$

I.e. $\alpha\,th\,A$ is the $\alpha$-th element of $A$ in increasing well-order.

### 1.3 The $\mathbb{NS}$ set

1. $1\,th,\mathbb{NS} = 0$
2. $(\alpha+1)\,th\,\mathbb{NS} = \alpha\,th\,\mathbb{NS} + \dfrac{1}{f(\alpha)}$
3. $\alpha\,th\,\mathbb{NS} = \lim_{i\to\infty} expand(\alpha,i)\,th\,\mathbb{NS}$, when $\lnot\exists\beta\;(\alpha=\beta+1)$ ($\alpha$ is a limit)

Hence $\mathbb{NS} = \{a_\alpha \mid \alpha\in On,\ \alpha\ge 1\}$ where $a_\alpha = \alpha\,th\,\mathbb{NS}$:
- $a_1 = 0$
- $a_{\alpha+1} = a_\alpha + 1/f(\alpha)$ ($\alpha\ge 1$)
- $a_\lambda = \sup_i a_{expand(\lambda,i)}$ ($\lambda$ a limit)

$\mathbb{NS}$ is a sequence of **reals indexed by ordinals** (the successor step $1/f(\alpha)$ shrinks, hence "nonlinear successor").

### 1.4 $\psi(x)$

$$ \psi(x)\,th\,\mathbb{NS} = x $$

I.e. $\psi(x)$ is the ordinal index of the real $x$ (which must belong to $\mathbb{NS}$) within the $\mathbb{NS}$ sequence — an inverse enumeration / collapsing function.

### 1.5 The parameterized family $n,m\text{-}Ns$

The source article also gives a parameterized generalization $n,m\text{-}Ns$ (same creators as Ns). Its auxiliary function replaces the constant $2$ in Ns by the parameters $n,m$:

$$ f(\alpha) = \begin{cases}
  1 & \alpha = 0 \\
  f(\beta)\cdot n & \alpha = \beta + 1 \\
  f(expand(\alpha,m)) & \lnot\exists\beta\;(\alpha=\beta+1)\quad(\alpha\text{ is a limit})
\end{cases} \qquad (\alpha\in On) $$

The general "$\alpha\,th\,A$" rule, the three $\mathbb{NS}$ rules, and $\psi(x)$ are **identical** to Ns; only $f$ is replaced by the parameterized version. Note the two parameters' division of labor:
- **$n$** appears both in the successor step ($\cdot n$) and in the fundamental-sequence index of the $\mathbb{NS}$ limit line, $expand(\alpha,n)$;
- **$m$** appears only in the limit branch of $f$, $expand(\alpha,m)$.

Thus **Ns is the $(n,m)=(2,2)$ special case**. The two fundamental-sequence dependencies (§2) become, for $n,m\text{-}Ns$: **$f$-limit uses $expand(\alpha,m)$, $\mathbb{NS}$-limit uses $expand(\alpha,n)$**, both pluggable (§11).

## 2. Cross-notation fundamental-sequence dependency (design.md §11)

Both `Ns` and `n,m-Ns` depend on **another ordinal notation's fundamental sequence** (design.md §11):

- **Ns ($(n,m)=(2,2)$)**: two dependencies
  1. the limit branch of $f$ uses `expand(α,2)`;
  2. the limit-element definition uses `expand(α,i)\,th\,\mathbb{NS}` (the whole fundamental sequence).
- **n,m-Ns**: two dependencies, each using a different parameter
  1. the limit branch of $f$ uses `expand(α,m)`;
  2. the limit-element definition uses `expand(α,n)\,th\,\mathbb{NS}` (the whole fundamental sequence).

Per the project's §11 principle: this *dependent part* must **support multiple ordinal notations** (the fundamental-sequence source is pluggable), not be hardwired to a single one. At implementation time, `expand(α,·)` should be a parameter / generic constraint chosen by the caller (e.g. `core/Ordinal`'s unified expansion, or some specific notation); for $n,m\text{-}Ns$, $n,m$ must also be carried as notation parameters.

## 3. Implementation notes (DONE)

- **Module placement**: implemented under `ordinal/ns/`, hosting both `Ns` and `n,m-Ns` (`Ns` = $(n,m)=(2,2)$). Taxonomy: `family()=Ordinal`, `subfamily()="ns"`, `style()="real_sequence"`.
- **Real arithmetic (accumulated weight fraction)**: a new `core/Rational.hpp` carries exact rationals and a new `WeightedFractionSum` type (the accumulated weight-fraction form: `int + w1/d1 + w2/d2 + ...`, e.g. `2 + 5/11 + 62/111`). For a finite index $\alpha=k\ge 1$ the exact value is
  $$a_k = \sum_{\beta=1}^{k-1}\frac{1}{n^{\beta}}$$
  ($n=2$ gives Ns: $a_1=0,\;a_2=1/2,\;a_3=3/4,\;a_4=7/8\ldots$; here the int part is 0 and each weight $w=1$). Limit indices need a fundamental sequence and are **not auto-computed** (below).
- **Pluggable FS source (§11)**: `expand` delegates to `core::Ordinal::expand` — the project's §11 pluggable fundamental-sequence source. Finite / successor indices have no FS, so `expand` throws `UnsupportedOperation` (exactly the "this can't be auto-computed" case the user noted).
- **Supported ops**: `FromString` / `ToString` (LaTeX, emits `α th \mathbb{NS}`), `Expand` (above), `Compare` (compares $\alpha$ directly, since $a_\alpha$ is strictly increasing in $\alpha$); `Normalize` / `Successor` are not defined by the article, so not implemented.
- **Creator / Version**: `creator()="送到本到, nnc"`, `version()="1"` (filled per the user's confirmation).
- **Accumulated weight-fraction views (finite $\alpha$ only)**: `value()` returns the exact `Rational`; `accumulated_weight_fractions()` returns a `WeightedFractionSum` (int part + (weight,denominator) list); `to_fraction_string()` renders `1/2 + 1/4 + …` (general form `2 + 5/11 + 62/111`); limit indices throw `std::domain_error` (not auto-computed).

## 4. Implementation file list

- `include/googology/core/Rational.hpp` — exact rational (gcd-reduced) + `WeightedFractionSum` (accumulated weight fraction: `int + w1/d1 + …`).
- `include/googology/notations/real_sequence/ns/Ns.hpp` + `src/notations/real_sequence/ns/Ns.cpp` — the `Ns` class (parameters `n_`,`m_`; `Ns` = `(2,2)`); symbolic `to_string()` + accumulated weight-fraction views (`accumulated_weight_fractions()` / `to_fraction_string()`) + `core::Ordinal`-delegated `expand` / `compare`. NOTE: `Ns` belongs to `Family::RealSequence` (an ordinal-indexed real sequence), it is NOT an ordinal notation, hence the directory is `real_sequence/` not `ordinal/`.
- `tests/unit/test_ns.cpp` — metadata, expression round-trip, accumulated weight-fraction values (incl. the `2 + 5/11 + 62/111` form), `expand`, `compare` assertions (all passing).
- `src/main.cpp` — registers `ns` and demonstrates it.
