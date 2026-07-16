# ε_pSS / ε_ωSS (difference-type ordinal sequence notations, two types in one module)

> One-line definition: both come from the same article ("epsilon_nSS & epsilon_omegaSS.md") and are **difference-type natural-number sequence** ordinal notations inheriting `OrdinalNotation`, sharing `baseVal_=1` and the same `expandLen_` / `compare` / `normalize` algorithm. The only difference is the per-step added amount: ε_pSS carries a parameter `p` (capped), while ε_ωSS has no `p` and adds the full gap `q` (unbounded) — it is the ω-limit of ε_pSS. This library performs **only symbolic expansion and LaTeX serialization — it never evaluates**.

Header: `include/googology/notations/ordinal/sequence/difference/epsilon_ss/EpsilonSS.hpp`
Namespace: `googology::ordinal`
Two types in one module: `EpspSS` (ε_pSS) / `EpsOmegaSS` (ε_ωSS).

---

## 1. Taxonomy

| Interface | `EpspSS` returns | `EpsOmegaSS` returns |
|-----------|------------------|----------------------|
| `name()` | `"epsilon_p_ss"` | `"epsilon_omega_ss"` |
| `family()` | `Family::Ordinal` | `Family::Ordinal` |
| `subfamily()` | `"sequence"` | `"sequence"` |
| `style()` | `"difference"` | `"difference"` |

**Category**: **ordinal notation (ordinal)**, difference-type natural-number sequence sub-family. Both types share one module and base `OrdinalNotation` → `Notation`, carrying standard-form logic.

---

## 2. Creator / Version

| Interface | `EpspSS` | `EpsOmegaSS` |
|-----------|----------|--------------|
| `creator()` | `"zahin"` | `"zahin"` |
| `version()` | `"1"` | `"1"` |

---

## 3. Capabilities

Both classes have the same `capabilities()`, declaring support for:

- `Op::FromString` ✔
- `Op::ToString` ✔
- `Op::Normalize` ✔
- `Op::Compare` ✔
- `Op::Expand` ✔
- `Op::ExpandTo` ✔
- `Op::Successor` ✔ (declares the §12 successor clause; predicate is `isSuccessor()`)

> `compare` is not in the article, but the user specifies that under standard form the notations admit lexicographic comparison (consistent with PrSS), valid only when both operands are in standard form; cross-type comparison throws `NotComparable`.

---

## 4. Inherited base interface (same as PrSS, see PrSS doc §4)

- `Notation`: `name` / `family` / `subfamily` / `style` / `creator` / `version` / `capabilities` / `can` / `string_to_it` / `to_string` / `compare` (overridden) / `expand` (overridden) / `expand_to` (overridden) / `comparable` / `print` / `operator<<` / `reduce`.
- `OrdinalNotation`: `virtual void normalize()` / `bool isSuccessor() const` (`baseVal_=1`; a sequence ending in 1 is a successor) / `bool is_standard() const` / `clone()` (overridden) / `roots()` (overridden).

> The **UMBRELLA definition** of standard form is the same as PrSS: "a standard expression ⇔ obtainable from a limit expression by finitely many expansions + taking a prefix." The user-specified limit expression for `epsilon_ss` is `(1, n)`, so the canonical starter is `(1, a_2)`.

---

## 5. This notation's own API (core)

### 5.1 `EpspSS` (ε_pSS)

#### Construction

```cpp
EpspSS();                                 // baseVal_ = 1
explicit EpspSS(BigInt p);                // set parameter p (cap on added amount), baseVal_ = 1
explicit EpspSS(const std::string& s);    // parse, baseVal_ = 1
EpspSS(BigInt p, const std::string& s);   // set p and parse, baseVal_ = 1
```

#### Parameter

```cpp
void   set_p(BigInt p);                   // set parameter p
BigInt p() const;                         // read parameter p
```

#### Overridden interface

```cpp
std::string name() const override;        // "epsilon_p_ss"
Family      family() const override;      // Family::Ordinal
std::string subfamily() const override;   // "sequence"
std::string style() const override;       // "difference"
std::string creator() const override;     // "zahin"
std::string version() const override;     // "1"

Capabilities capabilities() const override;

void        string_to_it(const std::string& s) override;
std::string to_string() const override;

EpspSS& expand(BigInt n) override;        // expand n steps IN PLACE, returns *this
EpspSS& expand_to(BigInt len) override;   // -> expandLen(A, len), returns *this

EpspSS operator[](BigInt n) const;        // A[n] = n-th fundamental-sequence term; does NOT mutate *this (returns copy)

static EpspSS limit(BigInt n);            // the n-th term of limit expression LIMIT=(1,ω)
static EpspSS master_limit();             // the master limit expression LIMIT (marked is_master_limit_)

int compare(const Notation& other) const override;   // lexicographic; cross-type throws NotComparable

OrdinalNotation* clone() const override;  // return new EpspSS(*this);
std::vector<std::shared_ptr<OrdinalNotation>> roots() const override;  // { master_limit() }

friend std::istream& operator>>(std::istream&, EpspSS&);
```

### 5.2 `EpsOmegaSS` (ε_ωSS)

#### Construction

```cpp
EpsOmegaSS();                             // baseVal_ = 1
explicit EpsOmegaSS(const std::string& s);// parse, baseVal_ = 1
```

#### Overridden interface

```cpp
std::string name() const override;        // "epsilon_omega_ss"
Family      family() const override;      // Family::Ordinal
std::string subfamily() const override;   // "sequence"
std::string style() const override;       // "difference"
std::string creator() const override;     // "zahin"
std::string version() const override;     // "1"

Capabilities capabilities() const override;

void        string_to_it(const std::string& s) override;
std::string to_string() const override;

EpsOmegaSS& expand(BigInt n) override;    // expand n steps IN PLACE, returns *this
EpsOmegaSS& expand_to(BigInt len) override; // -> expandLen(A, len), returns *this

EpsOmegaSS operator[](BigInt n) const;    // A[n]; does NOT mutate *this (returns copy)

static EpsOmegaSS limit(BigInt n);        // LIMIT=(1,ω) n-th term
static EpsOmegaSS master_limit();         // the master limit expression LIMIT

int compare(const Notation& other) const override;
OrdinalNotation* clone() const override;  // return new EpsOmegaSS(*this);
std::vector<std::shared_ptr<OrdinalNotation>> roots() const override;
friend std::istream& operator>>(std::istream&, EpsOmegaSS&);
```

**Behavior notes (both types)**
- `expand` / `expand_to` / `normalize`: modify `*this` **in place**; `expand`/`expand_to` return a self reference.
- `operator[]`: returns a copy and does **not** mutate `*this`. Repeated indexing is safe.
- `expand_to(len)` is equivalent to `expandLen(A, len)`.
- Exceptions: `compare` throws `NotComparable` for cross-type arguments; an out-of-range index is left as written and a defensive guard throws (no silent wrap); `normalize` leaves `*this` unchanged on a stall or non-standard input.

---

## 6. Examples (C++ snippets)

```cpp
#include "googology/notations/ordinal/sequence/difference/epsilon_ss/EpsilonSS.hpp"
#include <iostream>
using namespace googology;
using namespace googology::ordinal;

// Example 1: ε_pSS with parameter p, then expand
EpspSS a(5, "(1,3)");                     // p=5, parse sequence
std::cout << a.to_string() << "\n";
a.expand(2);
std::cout << a.to_string() << "\n";

// Example 2: ε_ωSS fundamental-sequence index (original unchanged)
EpsOmegaSS b("(1,2)");
EpsOmegaSS b1 = b[1];                      // 1st fundamental-sequence term; b unchanged
std::cout << b1.to_string() << "\n";

// Example 3: normalize + standard-form test + same-type compare
EpspSS c("(1,3,2)");
c.normalize();
std::cout << std::boolalpha << c.is_standard() << "\n";
EpspSS d("(1,3,1)");
int r = c.compare(d);                     // lexicographic; cross-type (e.g. vs Prss) throws NotComparable
```

---

## 7. Design notes

- **A notation is an expression; it is never evaluated**: `to_string()` only prints LaTeX; no `Evaluate` / `ToOrdinal`.
- **Same article = same type = same module**: ε_pSS and ε_ωSS are from one article, so they live together in the `epsilon_ss` module, both inherit `OrdinalNotation`, share `baseVal_=1` and the same algorithm; the only difference is the per-step added amount (ε_pSS capped at `p`, ε_ωSS full).
- **Standard form**: umbrella definition same as PrSS; `normalize()` is the normalization action, `is_standard()` is the predicate, `isSuccessor()` is the independent successor test (sequence ending in 1 is a successor); the user-specified limit expression is `(1, n)`.
- `compare` is a well-defined sequence lexicographic order under standard form; cross-type throws `NotComparable`.
- **Faithful policy (no added interpretation)**: article formulas implemented verbatim; out-of-range indices are left as written and a defensive guard throws rather than silently wrapping.

> Footnote: all function signatures are taken from the header `include/googology/notations/ordinal/sequence/difference/epsilon_ss/EpsilonSS.hpp`.
