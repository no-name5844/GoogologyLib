# Ns / n,m-Ns (non-linear successor, real-sequence notation)

> One-line definition: expresses the real `x = α th NS` as an **ordinal-indexed REAL sequence** (Ns is the `(n,m)=(2,2)` special case). The library keeps the symbolic expression `α th NS` and exposes the exact value for FINITE index α via an **accumulated weight fraction** (integer + w₁/d₁ + …). The library **never auto-computes a value for a limit index**.

Header: `include/googology/notations/real_sequence/ns/Ns.hpp`
Namespace: `googology::real_sequence`

---

## 1. Taxonomy

| Interface | Returns |
|-----------|---------|
| `name()` | `"ns"` |
| `family()` | `Family::RealSequence` |
| `subfamily()` | `"ns"` |
| `style()` | `"real_sequence"` |

**Category**: **real-sequence notation (real_sequence)** — an ordinal-indexed real sequence (`α th NS`, a real). It is **NOT an ordinal notation** and **NOT a large-number notation**. Carved out of `ordinal` into `real_sequence` on 2026-07-16 per the user's directive. Therefore it has **no** `normalize` / `is_standard` / `isSuccessor` / `limit` / `roots` ordinal methods.

---

## 2. Creator / Version

| Interface | Returns |
|-----------|---------|
| `creator()` | `"送到本到, nnc"` (per the Creator metadata in `ns.zh.md` / `ns.en.md`) |
| `version()` | `"1"` |

---

## 3. Capabilities

`capabilities()` declares support for:

- `Op::FromString` ✔
- `Op::ToString` ✔
- `Op::Expand` ✔ (delegates to `core::Ordinal::expand`, see §5.3)
- `Op::Compare` ✔ (reduces to comparing the ordinal index α, since `a_α` is strictly increasing in α)

**Not supported** (calling the corresponding interface throws an exception or `UnsupportedOperation`):
- `Op::Normalize` / `Op::Successor` ✘ —— not an ordinal notation; no standard-form / successor concept, so `normalize`/`isSuccessor` etc. **do not exist**.
- A finite / successor index has no fundamental sequence, so `expand` throws `UnsupportedOperation` for such indices (the "can't auto-compute" case the user noted).

---

## 4. Inherited base interface (`Notation`)

Ns inherits directly from `googology::Notation` (**not** `OrdinalNotation`), so:

- Has: `name` / `family` / `subfamily` / `style` / `creator` / `version` / `capabilities` / `can` / `string_to_it` / `to_string` / `compare` (overridden) / `expand` (overridden) / `comparable` / `print` / `operator<<` / `reduce`.
- **Does NOT have** (only `OrdinalNotation` does): `normalize` / `is_standard` / `isSuccessor` / `clone` / `roots` / `master_limit` / `limit`.

> The ordinal index α is stored internally as `core::Ordinal` (`core/Ordinal.hpp`, a value type that is **not** a `Notation` taxon). `expand(k)` delegates to `α.expand(k)` — the project's §11 pluggable fundamental-sequence source.

---

## 5. This notation's own API (core)

### 5.1 Members

```cpp
Ordinal alpha_;     // ordinal index α ("α th NS")
BigInt  n_ = 2;     // successor-step factor & NS-limit FS index
BigInt  m_ = 2;     // f-limit FS index
```

### 5.2 Construction & parameters

```cpp
Ns();                                     // alpha_ = Ordinal::fromInt(1) (default n_=m_=2)
explicit Ns(const std::string& s);        // parse, default n_=m_=2
Ns(BigInt n, BigInt m);                   // set n,m, alpha_ = fromInt(1)
Ns(BigInt n, BigInt m, const std::string& s); // set n,m and parse

void setParams(BigInt n, BigInt m);       // set n, m
std::pair<BigInt, BigInt> params() const; // read (n, m)
bool isDefaultParams() const;             // true iff n_==2 && m_==2 (the pure-Ns special case)
```

### 5.3 Overridden interface

```cpp
std::string name() const override;        // "ns"
Family      family() const override;      // Family::RealSequence
std::string subfamily() const override;   // "ns"
std::string style() const override;       // "real_sequence"
std::string creator() const override;     // "送到本到, nnc"
std::string version() const override;     // "1"

Capabilities capabilities() const override;

void string_to_it(const std::string& s) override;  // parse expression
std::string to_string() const override;            // expression "α th NS" (LaTeX-ish)

Ns& expand(BigInt k) override;            // delegate to core::Ordinal::expand (§11 pluggable FS)
                                          // finite / successor index has no FS -> throws UnsupportedOperation
```

### 5.4 Comparison

```cpp
int compare(const Notation& other) const override;  // reduces to comparing index α; non-Ns argument throws NotComparable
```

Since `a_α` is strictly increasing in α (each step adds a positive term `1/f(β)`), comparing two Ns is equivalent to comparing their ordinal indices α. A non-`Ns` argument throws `NotComparable`.

### 5.5 Accumulated weight-fraction views (meaningful only for FINITE α)

```cpp
Rational value() const;                   // exact rational value x = α th NS for finite α
                                           // throws std::domain_error for limit indices (not auto-computed)
WeightedFractionSum accumulated_weight_fractions() const; // accumulated weight fraction: int + w1/d1 + w2/d2 + ...
std::string to_fraction_string() const;   // rendered sum, e.g. "1/2 + 1/4"; "0" for the first element
```

- `value()`: returns the exact `Rational`. For a **limit index** it throws `std::domain_error` (needs a fundamental sequence; the library does not auto-compute). See `Rational.hpp`: `Rational(num, den)` auto-reduces, `to_string()` outputs `"num/den"` or an integer.
- `accumulated_weight_fractions()`: returns `WeightedFractionSum{ intPart, terms }` (each term is a `(weight, denominator)` pair, denominator > 0). For finite α the integer part is 0 and each weight is 1 (terms `1/n^β`). Its `.value()` aggregates to a `Rational`.
- `to_fraction_string()`: renders as `int + w1/d1 + ...` (e.g. `"2 + 5/11 + 62/111"`); the `α=1` first element outputs `"0"`.

**Behavior notes**
- `expand`: delegates to `core::Ordinal::expand`, modifies `*this` **in place** and returns a self reference; but a finite / successor index has no fundamental sequence, so it throws `UnsupportedOperation`.
- `value()` / `accumulated_weight_fractions()` / `to_fraction_string()`: **read-only**, do not mutate `*this`; defined only for finite α; `value()` throws `std::domain_error` for a limit index.
- Exceptions: `compare` throws `NotComparable` for cross-type arguments; `value()` throws `std::domain_error` for a limit index; `string_to_it` throws a parse exception on malformed input.

---

## 6. Examples (C++ snippets)

```cpp
#include "googology/notations/real_sequence/ns/Ns.hpp"
#include <iostream>
using namespace googology;
using namespace googology::real_sequence;

// Example 1: value of a finite index (accumulated weight fraction)
Ns x("2");                                // α = 2 (default n=m=2 -> pure Ns)
std::cout << x.to_string() << "\n";       // expression "2 th NS" (LaTeX-ish)
std::cout << x.to_fraction_string() << "\n"; // e.g. "1/2 + 1/4"
Rational v = x.value();                   // exact rational
std::cout << v.to_string() << "\n";

// Example 2: parameterized n,m-Ns
Ns y(3, 2, "2");                          // n=3, m=2, α=2
std::cout << y.params().first << "," << y.params().second << "\n"; // 3,2
std::cout << y.to_fraction_string() << "\n";

// Example 3: compare (reduces to comparing α); limit-index value() throws domain_error
Ns a("3");
Ns b("5");
int r = a.compare(b);                     // compare indices -> a < b returns -1
// Ns lim("ω"); lim.value();             // limit index -> throws std::domain_error
```

---

## 7. Design notes

- **A notation is an expression; it is never evaluated**: `to_string()` only prints the `α th NS` symbol; the library has **no** `Evaluate` / `ToOrdinal` numeric-evaluation interface.
- **Ns is an ordinal-indexed real sequence, NOT an ordinal notation**: hence it has **no** `normalize` / `is_standard` / `isSuccessor` / `limit` / `roots`. The umbrella standard-form definition applies only to ordinal notations.
- **Accumulated weight fraction expresses finite-index values**: the user specifies the exact `x = α th NS` for finite α as `int + w1/d1 + ...` (e.g. `"2 + 5/11 + 62/111"`). In Ns the integer part is 0 and each weight is 1 (terms `1/n^β`).
- **Limit indices are not auto-computed**: `value()` throws `std::domain_error` for a limit index; `expand` throws `UnsupportedOperation` for finite / successor indices (no fundamental sequence) — both correspond to the user's "can't auto-compute" case. `expand` delegates to `core::Ordinal::expand` (§11 pluggable FS source).
- `compare` reduces to comparing the ordinal index α (strictly increasing sequence); cross-type throws `NotComparable`.
- **Two types in one module**: Ns and n,m-Ns share the `ns` module; Ns is the `(n,m)=(2,2)` special case (detectable via `isDefaultParams()`).

> Footnote: all function signatures are taken from the header `include/googology/notations/real_sequence/ns/Ns.hpp`. `Rational` / `WeightedFractionSum` are defined in `include/googology/core/Rational.hpp`.
