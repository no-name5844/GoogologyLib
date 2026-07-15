# Ns (Nonlinear successo · nonlinear successor)

> **Source / reference article**: `study/notations/Ns.md`
> **Creator**: the article is signed `by 送到本到(UTF-8)`, which looks like mojibake / an encoding error; **to be confirmed by the user in the planning template** (left empty for now, per design.md §3c).
> **Version**: the article gives no explicit version number (TBD).
> **Reference**: the same source article defines the auxiliary function $f$, the set $\mathbb{NS}$, and $\psi(x)$.

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

1. $1st,\mathbb{NS} = 0$
2. $(\alpha+1)\,th\,\mathbb{NS} = \alpha\,th\,\mathbb{NS} + \dfrac{1}{f(\alpha)}$
3. $\alpha\,th\,\mathbb{NS} = \lim_{i\to\infty} expand(\alpha,i)\,th\,\mathbb{NS}$, when $\lnot\exists\beta\;(\alpha=\beta+1)$ ($\alpha$ is a limit; the original `th A` is a typo for `th NS`)

Hence $\mathbb{NS} = \{a_\alpha \mid \alpha\in On\}$ where
- $a_0 = 0$
- $a_{\alpha+1} = a_\alpha + 1/f(\alpha)$
- $a_\lambda = \sup_i a_{expand(\lambda,i)}$ ($\lambda$ a limit)

$\mathbb{NS}$ is a sequence of **reals indexed by ordinals** (the successor step $1/f(\alpha)$ shrinks, hence "nonlinear successor").

### 1.4 $\psi(x)$

$$ \psi(x)\,th\,\mathbb{NS} = x $$

I.e. $\psi(x)$ is the ordinal index of the real $x$ (which must belong to $\mathbb{NS}$) within the $\mathbb{NS}$ sequence — an inverse enumeration / collapsing function.

## 2. Cross-notation fundamental-sequence dependency (design.md §11)

`Ns` depends on **another ordinal notation's fundamental sequence** in **two places**:
1. the limit branch of $f$ uses `expand(α,2)`;
2. the limit-element definition uses `expand(α,i)\,th\,\mathbb{NS}` (the whole fundamental sequence).

Per the project's §11 principle: this *dependent part* must **support multiple ordinal notations** (the fundamental-sequence source is pluggable), not be hardwired to a single one. At implementation time, `expand(α,·)` should be a parameter / generic constraint chosen by the caller (e.g. `core/Ordinal`'s unified expansion, or some specific notation).

## 3. Implementation notes / pending

- **Module placement**: `Ns` yields an "ordinal-indexed real sequence"; it belongs neither to `number/` nor to a pure `ordinal/` notation. Proposed: `ordinal/ns/` (new subfamily), or a new taxonomy class (e.g. `ordinal_sequence`); **pending user confirmation**.
- **Real arithmetic**: elements are reals (rationals); a rational / real representation is needed (the library currently uses only `BigInt`).
- **Pluggable FS source**: the `expand` dependency must be pluggable (§11).
- **Supported ops (proposed, TBD)**: `FromString` / `ToString` (LaTeX), `Expand` (enumerate $a_\alpha$), `Compare` (reals are comparable); `Normalize` / `Successor` are not defined by the article, so not implemented.
- **Creator / Version**: the article's signature looks like mojibake; to be filled once confirmed as `送到本到` or the true name (design.md §3c).
