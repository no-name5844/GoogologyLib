# PrSS (Primitive Sequence System, difference type)

> One-line definition: a difference-type natural-number sequence notation `A=(a_1,...,a_n)` with `a_{i+1}-a_i ≤ 1`, `a_1=0`, and non-increase after a plateau. This library performs **only symbolic expansion and LaTeX serialization — it never evaluates**.

Header: `include/googology/notations/ordinal/sequence/difference/prss/Prss.hpp`
Namespace: `googology::ordinal`

---

## 1. Taxonomy

| Interface | Returns |
|-----------|---------|
| `name()` | `"prss"` |
| `family()` | `Family::Ordinal` |
| `subfamily()` | `"sequence"` |
| `style()` | `"difference"` |

**Category**: **ordinal notation (ordinal)**, difference-type natural-number sequence sub-family. `Prss` inherits `OrdinalNotation` → `Notation`, so it carries standard-form logic.

---

## 2. Creator / Version

| Interface | Returns |
|-----------|---------|
| `creator()` | `""` (not overridden; base-class default empty — **to be annotated later**) |
| `version()` | not overridden; base-class default `""` |

> Note: the PrSS header does not override `creator()` / `version()`, so both return empty strings. The naming attribution is left for the user's later "planned module".

---

## 3. Capabilities

`capabilities()` declares support for:

- `Op::FromString` ✔
- `Op::ToString` ✔
- `Op::Normalize` ✔
- `Op::Compare` ✔
- `Op::Expand` ✔
- `Op::ExpandTo` ✔
- `Op::Successor` ✔ (declares the §12 successor clause; the concrete predicate is `isSuccessor()`)

Operations not in the set throw `UnsupportedOperation` when called (all of the above are overridden, so the base implementations are usable).

---

## 4. Inherited base interface

### 4.1 From `Notation` (generic)

`name` / `family` / `subfamily` / `style` / `creator` / `version` / `capabilities` / `can(Op)` / `string_to_it` / `to_string` / `compare` (overridden) / `expand` (overridden) / `expand_to` (overridden) / `comparable` / `print` / `operator<<` / `reduce`.

### 4.2 From `OrdinalNotation` (ordinal intermediate base)

```cpp
virtual void normalize();                 // NORMALIZE: rewrite *this IN PLACE to standard form
bool isSuccessor() const;                 // successor test (independent of standard form): a sequence is a successor iff it ends with baseVal_ (=0)
bool is_standard() const;                 // standard-form PREDICATE: runs the §12 generic decision engine
virtual OrdinalNotation* clone() const = 0;   // polymorphic copy (Prss overrides)
virtual std::vector<std::shared_ptr<OrdinalNotation>> roots() const = 0; // top-level limit expressions (Prss overrides)
```

> The **UMBRELLA definition** of standard form (shared by all ordinal notations): "an expression is a *standard expression* ⇔ it is obtainable from some *limit expression* by finitely many *expansions* (expand) and taking a *prefix of the fundamental sequence*."
>
> - `normalize()` = **verb**: the action of rewriting an expression into the standard form (per-notation algorithm; PrSS uses `baseVal_=0`).
> - `is_standard()` = **predicate**: tests whether `*this` is already in standard form.
> - `isSuccessor()` = successor test, **completely independent of standard form** (checks only whether the last element equals `baseVal_`).

---

## 5. This notation's own API (core)

### 5.1 Construction & parsing

```cpp
Prss() = default;
explicit Prss(const std::string& s);      // equivalent to default-construct then string_to_it(s)
```

### 5.2 Overridden interface

```cpp
std::string name() const override;        // "prss"
Family      family() const override;      // Family::Ordinal
std::string subfamily() const override;   // "sequence"
std::string style() const override;       // "difference"

Capabilities capabilities() const override;

void string_to_it(const std::string& s) override;  // parse sequence text
std::string to_string() const override;            // emit LaTeX

Prss& expand(BigInt n) override;          // expand n steps IN PLACE (n-th fundamental-sequence term), returns *this
Prss& expand_to(BigInt len) override;     // expand to length len, returns *this
```

### 5.3 Fundamental-sequence index (does NOT mutate `*this`)

```cpp
Prss operator[](BigInt n) const;          // A[n] = the n-th term of the fundamental sequence of the ordinal A denotes
```

Equivalent to `expand(A, n)`, but **does not modify** `*this` (returns a copy), so repeated `A[1]`, `A[2]`, … are safe.

### 5.4 §12 limit-expression API

```cpp
static Prss limit(BigInt n);              // the n-th term of limit expression LIMIT=(0,1,2,3,...)
static Prss master_limit();               // the master limit expression LIMIT itself (marked is_master_limit_)
```

`LIMIT = (0,1,2,3,...)`: `limit(0)=()`, `limit(1)=(0)`, `limit(2)=(0,1)`, …, `limit(n)=(0,1,...,n-1)`. Satisfies `LIMIT.expand(m) == limit(m)`; `is_standard()` seeds the §12 BFS via `roots()` with `LIMIT` (the master limit is the supremum of all standard Prss expressions).

### 5.5 Comparison

```cpp
int compare(const Notation& other) const override;   // lexicographic order over the sequence (ordinal order)
```

- Returns `-1/0/1` (`<` / `=` / `>`).
- A non-`Prss` argument (cross-type) throws `NotComparable`.
- The master limit compares as the **supremum** (greater than every finite sequence).

### 5.6 §12 standard-form decision support

```cpp
OrdinalNotation* clone() const override;  // return new Prss(*this);
std::vector<std::shared_ptr<OrdinalNotation>> roots() const override;  // { master_limit() }
```

**Behavior notes**
- `expand` / `expand_to` / `normalize`: all **in place**; `expand`/`expand_to` return a self reference, `normalize` returns nothing.
- `operator[]`: returns a copy and does **not** mutate `*this`.
- Exceptions: `compare` throws `NotComparable` for cross-type arguments; `normalize` leaves `*this` **unchanged** on a stall or non-standard input (does not throw, only reverts); `string_to_it` may throw a parse exception on malformed input.

---

## 6. Examples (C++ snippets)

```cpp
#include "googology/notations/ordinal/sequence/difference/prss/Prss.hpp"
#include <iostream>
using namespace googology;
using namespace googology::ordinal;

// Example 1: construct, expand, serialize (no evaluation)
Prss a("(0,1,2)");
std::cout << a.to_string() << "\n";       // LaTeX sequence
a.expand(3);
std::cout << a.to_string() << "\n";       // expanded form

// Example 2: fundamental-sequence index (original unchanged)
Prss b("(0,1,2)");
Prss b1 = b[1];                           // 1st fundamental-sequence term; b itself unchanged
std::cout << b1.to_string() << "\n";

// Example 3: normalize + standard-form test + compare
Prss c("(0,1,2,1,2)");                    // possibly non-standard
c.normalize();                            // normalize in place
bool ok = c.is_standard();                // predicate: is it in standard form?
std::cout << std::boolalpha << ok << "\n";
Prss d("(0,1,2,3)");
int r = c.compare(d);                     // lexicographic; cross-type compare throws NotComparable
```

---

## 7. Design notes

- **A notation is an expression; it is never evaluated**: `to_string()` only prints LaTeX; no `Evaluate` / `ToOrdinal`.
- **Standard form (noun)**: governed by the §12 umbrella definition; `normalize()` is the per-notation normalization action, `is_standard()` is the standard-form predicate, `isSuccessor()` is the successor test independent of standard form. PrSS uses `baseVal_=0`.
- **Limit expression / roots**: `master_limit()` is marked `is_master_limit_`; its `expand(m)==limit(m)` and is seeded by `roots()` as the §12 BFS start (supremum).
- `compare` is a **well-defined ordinal order** (sequence lexicographic order); cross-family / cross-type comparisons throw `NotComparable`. Large-number notations (Knuth/Conway) lack this capability.

> Footnote: all function signatures are taken from the header `include/googology/notations/ordinal/sequence/difference/prss/Prss.hpp`. `creator()` returns the empty base-class default; the attribution is to be annotated later by the user.
