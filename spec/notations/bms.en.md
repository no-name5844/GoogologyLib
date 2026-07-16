# BMS (Bashicu Matrix System) — multi-version computation rules

> **Source**: `study/notations/BMS.md`
> **Creator**: Bashicu (initial BM1); library `creator()` to be filled later per user plan (currently `""`, see `doc/api/bms.en.md`).
> **Version**: BM1=2014, BM2=2016, BM2.3/BM4=2018, BM3.3=2019; library distinguishes via subclasses, `version()` pending.
> **Notation class**: ordinal notation (matrix → ordinal via `Trans()`). Library expands only one step and never evaluates, so **termination / well-foundedness is deliberately ignored** (user directive: throw well-foundedness in the bin).
> **Library mapping**: base class `BMS` (§0 shared skeleton) + subclasses `BM4` / `BM1` / `BM3.3` (diff hooks `parentOf` / `ascensionDegree`); other versions not wired in because their $a_{k,m}$ exact formulas appear only in the basmat source, not public docs (see `doc/api/bms.en.md`).

Notation: column index $x$, row index $y$ (from $0$). $S=S_0S_1\cdots S_{X-1}$ ($X$ columns); column $S_x=(S_{x,0},\dots,S_{x,Y-1})$. $S_{x,y}$ is element at column $x$, row $y$. $S+T$ is column concatenation. Last column $S_{X-1}$; its lowest nonzero entry is at row $z=\max\{y\mid S_{X-1,y}>0\}$ (the LNZ).

---

# 0. Shared expansion skeleton (BM4 standard; shared by all versions)

# 0.1. Definition
A valid matrix (standard form) satisfies:
1. First column all $0$: $S_{0,y}=0$;
2. Each column non-increasing top-to-bottom: $S_{x,y}\ge S_{x,y+1}$;
3. Same row advances by at most $1$: $S_{x,y}\le \max_{p<x}S_{p,y}+1$ (empty max is $0$).

# 0.2. Expansion

## 0.2.1 Auxiliary functions
1. **Parent** $p_k(m)$: the largest $p<m$ satisfying
   $$S_{p,k}<S_{m,k}\quad\text{and (if }k>0\text{)}\quad a_{k-1,m}(p)=1$$
   where $a_{k-1,m}(p)=1\iff\exists c\ge0:(p_{k-1})^c(m)=p$ (column $p$ is an ancestor of column $m$ at row $k-1$). Convention: $(p_k)^0(m)=m$.
2. **Ascension degree** $a_{k,m}(t)$: for a bad-part column $m$ ($t$ is the bad-root column index),
   $$a_{k,m}(t)=\begin{cases}1,& \exists c\ge0:(p_k)^c(m)=t\\0,& \text{otherwise}\end{cases}$$
3. **Bad root** $r=p_z(X-1)$ (parent of the last column's LNZ); no bad root if the last column is all zero.
4. **Good / bad part**: $G=S_0+\cdots+S_{r-1}$, $B=S_r+\cdots+S_{X-2}$.
5. **Difference** $\Delta_k=\begin{cases}S_{X-1,k}-S_{r,k},& k<z\\0,& k\ge z\end{cases}$.
6. **Copy** $B^{(i)}$ column $m$: $B^{(i)}_{m,k}=B_{m,k}+a_{k,r+m}(r)\cdot\Delta_k\cdot i$.

## 0.2.2 expand function
$$
\begin{aligned}
\mathit{expand}((),n)&=n\\
\mathit{expand}(S+(0),n)&=\mathit{expand}(S,2n)\\
\mathit{expand}(S,n)&=\mathit{expand}\bigl(G+B^{(0)}+\cdots+B^{(n)},\;2n\bigr)\quad(\text{last column not all }0)
\end{aligned}
$$
Fundamental-sequence predecessor $FS_n(S)=G+B^{(0)}+\cdots+B^{(n-1)}$. Ordinal map:
$$
\mathit{Trans}()=0,\quad \mathit{Trans}(S+(0,\dots,0))=\mathit{Trans}(S)+1,\quad \mathit{Trans}(M)=\sup\{\mathit{Trans}(FS_i(M))\mid i\in\mathbb N\}.
$$
**Library convention**: `expand(n)` emits only the $n$-th fundamental column $FS_n(S)$ (one-step expansion, consistent with Prss / EpspSS), without recursive iteration or evaluation.

---

# 1. BM1 (initial, 2014)
# 1.1. Definition — same as §0.1.
# 1.2. Expansion
## 1.2.1 Auxiliary functions
Parent $p_k(m)$ ($k>0$) **omits the upward-ancestor check** (the $a_{k-1,m}(p)=1$ condition is disabled), degenerating to a first-row-only test. Ascension degree is unified by the first row:
$$a_{k,m}(t)=a_{0,m}(t)\quad(\forall k)$$
## 1.2.2 expand function — same as §0.2.2.
Termination: no.

---

# 2. BM2 (2nd, 2016)
# 2.1. Definition — same as §0.1.
# 2.2. Expansion
## 2.2.1 Auxiliary functions
Introduces per-row ascension degrees, but the parent / ascension-degree rule differs from BM2.3 (bad-part ancestor-chain handling is wrong); exact per-line formula not given in public docs.
## 2.2.2 expand function — same as §0.2.2.
Termination: no.

---

# 3. BM2.1 (koteitan transitional)
# 3.1. Definition — same as §0.1.
# 3.2. Expansion
## 3.2.1 Auxiliary functions
Intermediate fix BM2 → BM2.3; exact $a_{k,m}$ rule in basmat source.
## 3.2.2 expand function — same as §0.2.2. Termination: not separately proven.

---

# 4. BM2.2 (koteitan transitional)
# 4.1. Definition — same as §0.1.
# 4.2. Expansion
## 4.2.1 Auxiliary functions — same as BM2.1.
## 4.2.2 expand function — same as §0.2.2. Termination: not separately proven.

---

# 5. BM2.3 (koteitan fix, 2018)
# 5.1. Definition — same as §0.1.
# 5.2. Expansion
## 5.2.1 Auxiliary functions
Uses §0.2.1 parent & ascension-degree rules (with upward-ancestor check, per-row independent). **Identical to BM4.**
## 5.2.2 expand function — same as §0.2.2 (= BM4). Termination: same as BM4.

---

# 6. BM3 (3rd, 2018)
# 6.1. Definition — same as §0.1.
# 6.2. Expansion
## 6.2.1 Auxiliary functions
Per-row ascension degrees (closer to BM4 than BM1), but the $k>0$ parent $p_k$ condition differs from BM4 (incomplete: ancestor-chain check inconsistent with BM4), misjudging $a_{k,m}$ on some bad-part columns.
## 6.2.2 expand function — same as §0.2.2.
Termination: no.

---

# 7. BM3.1 (Nish variant)
# 7.1. Definition — same as §0.1.
# 7.2. Expansion
## 7.2.1 Auxiliary functions — community fix of BM3; exact $a_{k,m}$ rule in basmat source.
## 7.2.2 expand function — same as §0.2.2. Termination: unproven.

---

# 8. BM3.2 (Nish variant)
# 8.1. Definition — same as §0.1.
# 8.2. Expansion
## 8.2.1 Auxiliary functions — same as BM3.1.
## 8.2.2 expand function — same as §0.2.2. Termination: unproven.

---

# 9. BM3.3 (Rpakr + ecl1psed, 2019)
# 9.1. Definition — same as §0.1.
# 9.2. Expansion
## 9.2.1 Auxiliary functions
Only difference from BM4 is the ascension degree. Let $t=$ the bad-root column $r$ (ascension-degree target). Bad-part columns split into two classes:
- **Columns whose row-$t$ entry is a descendant of $r$ (incl. $r$ itself)**: first $t-1$ rows have $a_{k,m}=1$ (same as BM4).
- **Columns whose row-$t$ entry is NOT a descendant of $r$**:
  $$\text{BM4}:\quad a_{k,m}=1\iff a_{k,\mathrm{parent}(m)}=1\ \land\ (\mathrm{parent}=r\ \lor\ \mathrm{parent}>r)$$
  $$\text{BM3.3}:\quad a_{k,m}=1\iff a_{k,\mathrm{parent}(m)}=1\ \land\ \mathrm{parent}>r$$
  (BM3.3 removes the "parent = bad root" clause.)
## 9.2.2 expand function — same as §0.2.2 (ascension degree per 9.2.1). Termination: unproven.
Example: $(0,0,0)(1,1,1)(2,1,0)(1,1,1)$ under BM3.3 expands with first term
$$(0,0,0)(1,1,1)(2,1,0)(1,1,0)\dots$$

---

# 10. BM4 (current standard, 2018-09-01)
BM4 = BM2.3 standardized; expansion rule is exactly the §0 skeleton.
# 10.1. Definition — same as §0.1.
# 10.2. Expansion
## 10.2.1 Auxiliary functions — same as §0.2.1.
## 10.2.2 expand function — same as §0.2.2.
Termination: claimed well-founded (unreviewed). Library ignores this.

---

# 11. PsiCubed2 version (community personal variant)
# 11.1. Definition — same as §0.1.
# 11.2. Expansion
## 11.2.1 Auxiliary functions — personal variant; exact $a_{k,m}$ rule on their blog; difference again in parent / ascension degree.
## 11.2.2 expand function — same as §0.2.2. Termination: unevaluated.

---

# 12. Idealized BMS series (community proposals)
Goal: make $A(1,1,1)\leftrightarrow\psi(C+\Omega_\omega)$ hold (tidier ordinal correspondence). Each proposal tweaks only ascension degree / bad-part test / parent check; none adopted as standard. Names: `Mar.19(1)`, `Mar.19(2)`, `BR+delta comp`, `UBRABC`, `UBRABC+parent check`, `UBRA+parent check`, `Rpakr Def.1`–`Def.5`. Computation rules in each proposal's original; termination all open.
