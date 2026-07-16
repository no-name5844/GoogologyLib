# BMS / BM4 / BM1 / BM3.3 (Bashicu Matrix System, ordinal notation)

> One-line definition: treats a column-written matrix of non-negative integers as an expression; recursion generates ordinals by one-step expanding the **last column** (fundamental-sequence predecessor $FS_n$). All versions share the §0 skeleton; the only differences are the **parent condition** and the **ascension degree $a_{k,m}$**. The library **never evaluates, only expands one step**, so termination / well-foundedness is deliberately ignored (user directive: throw well-foundedness in the bin).

Header: `include/googology/notations/ordinal/matrix/bms/BMS.hpp`
Impl: `src/notations/ordinal/matrix/bms/BMS.cpp`
Namespace: `googology::ordinal`

---

## 1. Taxonomy

| Interface | Return (BM4 shown; BM1 / BM3.3 differ in `name()`) |
|------|--------|
| `name()` | `"bm4"` (`BM1`→`"bm1"`, `BM3_3`→`"bm3.3"`) |
| `family()` | `Family::Ordinal` |
| `subfamily()` | `"bms"` |
| `style()` | `"matrix"` |

**Class**: **ordinal notation** — matrix maps to an ordinal via `Trans()` (see `spec/notations/bms.en.md` §0.2.2). `compare()` uses **syntax total order** (column-major lexicographic), for containers / sorting — **not** the real ordinal order (which needs `Trans()`, possibly non-terminating — binned).

---

## 2. Creator / Version

| Interface | Return |
|------|--------|
| `creator()` | `""` (to be filled per user plan; source credits Bashicu) |
| `version()` | `""` (pending; BM4=2018, BM1=2014, BM3.3=2019) |

> Subclasses each override `name()`; `creator()` / `version()` currently inherit the base default `""` (library convention: unknown, pending).

---

## 3. Capabilities

`capabilities()` declares support for:

- `Op::FromString` ✔ (`string_to_it` parses `(a,b,...)(c,d,...)`)
- `Op::ToString` ✔ (`to_string` serializes back to the same format)
- `Op::Expand` ✔ (`expand(n)` = the $n$-th fundamental column $FS_n$, in-place, returns `*this`)
- `Op::Compare` ✔ (syntax order; cross-type throws `NotComparable`)
- `Op::Normalize` ✔ (enforces §0.1 three conditions; if unsatisfied, leaves `*this` unchanged, library convention)

**Not supported**:
- `Op::Successor` ✘ — BMS has no simple-successor notion; `isSuccessor()` always returns `false` (not a virtual `OrdinalNotation` method, so no `override`).
- `Op::ExpandTo` ✘ — not implemented (single-step `expand` suffices).

> Check: `bms.can(Op::X)` (inherited `Notation::can`, wraps `capabilities().has(Op::X)`) returns `bool`; or `bms.capabilities().has(Op::X)`.

---

## 4. Inherited base interface (`OrdinalNotation` / `Notation`)

BMS derives from `googology::ordinal::OrdinalNotation` (→ `Notation`), thus:

- Owns: `name` / `family` / `subfamily` / `style` / `creator` / `version` / `capabilities` / `can` / `string_to_it` / `to_string` / `compare` (overridden) / `expand` (overridden) / `comparable` / `print` / `operator<<` / `reduce`.
- Ordinal-notation members (virtual in `OrdinalNotation`, relevant ones overridden by BMS): `normalize` / `is_standard` / `isSuccessor` (non-virtual, BMS supplies its own) / `clone` / `roots`.

> `is_standard()` is **`virtual`** (matches "standard-form judgment is per-notation"): BMS uses the §0.1 syntactic check directly; `OrdinalNotation`'s default §12 engine is overridden here.

---

## 5. The notation's own API (core)

### 5.1 Public members (base `BMS`)

```cpp
std::vector<std::vector<BigInt>> cols_;   // column-major matrix: cols_[x] = column x (top-to-bottom)
```

All versions share one matrix store and the §0 skeleton; differences are exposed through two **`virtual` hooks**:

```cpp
// Parent p_k(m): largest p<m with S_{p,k}<S_{m,k}, and (k==0 ? true : ascensionDegree(k-1,m,p)==1)
virtual int  parentOf(int k, int m) const;
// Ascension degree a_{k,m}(t): true iff t is an ancestor of m under p_k (incl. m itself)
virtual bool ascensionDegree(int k, int m, int t) const;
```

### 5.2 Overridden interfaces

```cpp
std::string name() const override;            // per subclass: "bm4" / "bm1" / "bm3.3"
Family      family() const override;           // Family::Ordinal
std::string subfamily() const override;    // "bms"
std::string style() const override;          // "matrix"

Capabilities capabilities() const override;

void  string_to_it(const std::string& s) override;  // parse (a,b,...)(c,d,...) format
std::string to_string() const override;            // serialize back to same format

BMS& expand(BigInt n) override;          // FS_n: in-place rewrite *this, return self (covariant)
int  compare(const Notation& other) const override;  // syntax order; non-BMS arg throws NotComparable

void normalize() override;                 // enforce §0.1; if unsatisfied, keep *this
bool is_standard() const override;         // §0.1 three conditions (syntactic) directly
bool isSuccessor() const;                  // non-virtual: always false
OrdinalNotation* clone() const override = 0;        // per subclass
std::vector<std::shared_ptr<OrdinalNotation>> roots() const override;  // same-type empty-matrix seed
```

### 5.3 Subclasses (version differences)

| Class | Difference (vs BM4) |
|----|------|
| `BM4` | Only fully-specified rule; `parentOf` keeps upward-ancestor check, `ascensionDegree` per-row independent ancestor chain (`(p_k)^c(m)=t`). |
| `BM1` | `parentOf` **drops the ancestor check** (first-row-only); `ascensionDegree` unified by first row $a_{k,m}(t)=a_{0,m}(t)$. Termination: no. |
| `BM3_3` | `parentOf` same as BM4; `ascensionDegree` drops the "parent = bad root" clause ($a_{k,m}=1\iff a_{k,\mathrm{parent}(m)}=1\land\mathrm{parent}>r$). Termination: unproven. |

> Other versions (BM2 / BM2.1 / BM2.2 / BM3 / BM3.1 / BM3.2 / PsiCubed2 / Idealized) are **not implemented** because their $a_{k,m}$ exact formulas "appear only in the basmat source, not public docs"; the header notes them as pending, to be wired in later as subclasses (same `parentOf` / `ascensionDegree` hooks).

### 5.4 Comparison semantics

```cpp
int compare(const Notation& other) const override;  // column-major lexicographic (syntax order)
```
- Same type (both some BMS subclass): compare column-major (column 0 row-by-row, then column 1, …) lexicographically; differing lengths resolved by present columns, out-of-bounds rows treated as $0$. Returns $-1/0/+1$.
- **Cross-type** (argument is not a `BMS` subclass): throws `NotComparable` (`compare` is not the real ordinal order; library never compares across families).

### 5.5 Standard form / normalization

- `is_standard()`: **syntactic** §0.1 three-condition check (first col all 0 / each col non-increasing / same row advances ≤1), independent of expansion or termination.
- `normalize()`: if §0.1 already holds, no-op; if not (invalid matrix), per library convention **leaves `*this` unchanged** (no throw, no collapse).
- `isSuccessor()`: always `false` (BMS has no simple-successor notion).

---

## 6. Examples (C++ snippets)

```cpp
#include "googology/notations/ordinal/matrix/bms/BMS.hpp"
#include <iostream>
using namespace googology;
using namespace googology::ordinal;

// Ex 1: BM4 one-step expansion (fundamental column FS_n)
BM4 a("(0,0,0)(1,1,1)(2,2,0)");
a.expand(1);
std::cout << a.to_string() << "\n";   // (0,0,0)(1,1,1)  == FS_1
BM4 b("(0,0,0)(1,1,1)(2,2,0)");
b.expand(2);
std::cout << b.to_string() << "\n";   // (0,0,0)(1,1,1)(2,1,1) == FS_2

// Ex 2: single-column decrement (last col has no parent, r<0 branch)
BM4 c("(5)");
c.expand(3);
std::cout << c.to_string() << "\n";   // (2)   (5 - 3)

// Ex 3: BM3.3 note-example first term
BM3_3 d("(0,0,0)(1,1,1)(2,1,0)(1,1,1)");
d.expand(1);
std::cout << d.to_string() << "\n";   // (0,0,0)(1,1,1)(2,1,0)(1,1,0)

// Ex 4: capabilities / standard form / cross-type compare
BM4 e("(0,0,0)(1,1,1)(2,2,0)");
std::cout << e.is_standard() << "\n";          // 1 (satisfies §0.1)
std::cout << e.capabilities().has(Op::Expand) << "\n";  // 1
bool threw = false;
try { BM4 x("(1)"); /* compare against non-BMS */ }
catch (const NotComparable&) { threw = true; }
```

---

## 7. Design notes

- **Notation is expression, never evaluated**: `to_string()` only prints the matrix symbol `(a,b,...)(c,d,...)`; the library has **no** `Evaluate` / `ToOrdinal` numeric interface. `expand(n)` emits only the $n$-th fundamental column $FS_n(S)$ (`spec/notations/bms.en.md` §0.2.2) — no recursive iteration, no collapse to a number.
- **Base class + subclass framework (user directive)**: base `BMS` implements the §0 shared skeleton (column-major store + §0.1 standard-form conditions + §0.2.1 shared helpers + §0.2.2 expand skeleton), exposing version differences through the two `virtual` hooks `parentOf` / `ascensionDegree`; `BM4` / `BM1` / `BM3_3` override the hooks as subclasses. Adding a version = adding a subclass.
- **Termination / well-foundedness: binned**: explicit user directive. The library only does "one-step expansion" (consistent with Prss / EpspSS), neither proving nor depending on termination. `compare()` uses **syntax order** (column-major lexicographic), not the real ordinal order; cross-type throws `NotComparable`.
- **Storage**: column-major; `S_{x,y}` = `get_(x,y)`, out-of-bounds rows treated as $0$ (columns may differ in height, short columns are 0 below). `BigInt` is an `int64_t` alias (only expansion index / parse params).
- **Unimplemented versions**: BM2 / BM2.1 / BM2.2 / BM3 / BM3.1 / BM3.2 / PsiCubed2 / Idealized — their $a_{k,m}$ exact formulas appear only in the basmat source, not public docs; the header notes them as pending, not yet wired in.

> Footnote: all signatures herein follow `include/googology/notations/ordinal/matrix/bms/BMS.hpp`; expansion algorithm details in `spec/notations/bms.en.md`.
