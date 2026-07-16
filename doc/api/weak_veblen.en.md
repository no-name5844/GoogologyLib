# WeakVeblen (weak Veblen-like notation, zahin)

> One-line definition: a weak-Veblen-like ordinal notation `A=(a_1@b_1, ..., a_n@b_n)` (`a_i,b_i∈On`, `b_i>b_{i+1}`), implemented strictly following the six expansion cases of the article `weak-Veblen-like notation.md`. This library performs **only symbolic expansion and LaTeX serialization — it never evaluates**.

Header: `include/googology/notations/ordinal/veblen/weakveblen/WeakVeblen.hpp`
Namespace: `googology::ordinal`

---

## 1. Taxonomy

| Interface | Returns |
|-----------|---------|
| `name()` | `"weak_veblen"` |
| `family()` | `Family::Ordinal` |
| `subfamily()` | `"veblen"` |
| `style()` | `"weak-like"` |

**Category**: **ordinal notation (ordinal)**, Veblen sub-family. But WeakVeblen derives **straight from `Notation`** (it does **not** inherit `OrdinalNotation`): its entries are **ordinal PAIRS `(a@b)`**, and the article defines **no standard form**, so — following the "write what the article says" principle — it adds no `normalize` / `is_standard` / `isSuccessor` / `roots` / `limit` sequence methods. Whether a standard-form concept belongs here is left to the user.

---

## 2. Creator / Version

| Interface | Returns |
|-----------|---------|
| `creator()` | `"zahin"` (article title is "zahin's weak-Veblen-like notation") |
| `version()` | `"1"` (article gives no explicit version) |

---

## 3. Capabilities

`capabilities()` declares support for:

- `Op::FromString` ✔
- `Op::ToString` ✔
- `Op::Compare` ✔ (the article §1.3 supplies `is_equal` / `is_greater` / `compare`)
- `Op::Expand` ✔

**Not supported** (calling the corresponding base-class interface throws `UnsupportedOperation`):
- `Op::ExpandTo` ✘ —— `expand_to` is not overridden in the header, so it falls through to the base class and throws `UnsupportedOperation`.
- `Op::Normalize` / `Op::Successor` ✘ —— the article defines no standard-form / successor clause, so they are not declared; `normalize` / `isSuccessor` etc. **do not exist** on this notation.

---

## 4. Inherited base interface (`Notation`)

WeakVeblen inherits directly from `googology::Notation` (**not** `OrdinalNotation`), so:

- Has: `name` / `family` / `subfamily` / `style` / `creator` / `version` / `capabilities` / `can` / `string_to_it` / `to_string` / `compare` (overridden) / `expand` (overridden) / `comparable` / `print` / `operator<<` / `reduce`.
- **Does NOT have** (only `OrdinalNotation` does): `normalize` / `is_standard` / `isSuccessor` / `clone` / `roots` / `master_limit` / `limit`.

> Note: the internal `Ordinal ord_` is the project's unified ordinal expression type (`core/Ordinal.hpp`, a value type that is **not** a `Notation` taxon). `expand` rewrites that `Ordinal` internally and returns `*this`; the `(a@b)` expansion (six cases) is implemented by `Ordinal::expandWV`, with ordinal arithmetic (`+1`, `^`, `expand(b,n)`) supplied by the `Ordinal` core.

---

## 5. This notation's own API (core)

### 5.1 Construction

```cpp
WeakVeblen() = default;                                  // default (no expression initialized)
explicit WeakVeblen(const std::string& s);               // parse "a1@b1, a2@b2, ..."
explicit WeakVeblen(const std::vector<std::pair<Ordinal, Ordinal>>& c); // build directly from (a@b) pairs
```

### 5.2 Overridden interface

```cpp
std::string name() const override;        // "weak_veblen"
Family      family() const override;      // Family::Ordinal
std::string subfamily() const override;   // "veblen"
std::string style() const override;       // "weak-like"
std::string creator() const override;     // "zahin"
std::string version() const override;     // "1"

Capabilities capabilities() const override;

void string_to_it(const std::string& s) override;   // parse "a1@b1, a2@b2, ..."
std::string to_string() const override;             // render the (a@b) expression

WeakVeblen& expand(BigInt n) override;   // rewrite internal Ordinal per the article's 6 cases, returns *this
```

### 5.3 Comparison

```cpp
int compare(const Notation& other) const override;   // article §1.3: returns 1(>), 0(=), -1(<)
```

- Defined on WV **standard-form** expressions; a non-`WeakVeblen` argument throws `NotComparable`.
- Implemented as lexicographic order (primary key = second coordinate `@b`, secondary = `@a`); see the verbatim implementation in `WeakVeblen.cpp`.

### 5.4 Fundamental-sequence index (does NOT mutate `*this`)

```cpp
WeakVeblen operator[](BigInt n) const;   // A[n] = the n-th fundamental-sequence term of the ordinal A denotes; equivalent to expand(A,n)
```

Returns a copy and does **not** mutate `*this`; repeated indexing is safe. Note `operator[]` is not a `Capability` bit — it is merely syntactic sugar over `expand`.

**Behavior notes**
- `expand`: rewrites the internal `Ordinal` **in place** on `*this` and returns a self reference.
- `expand_to`: not overridden, so calling it throws `UnsupportedOperation`.
- Exceptions: `compare` throws `NotComparable` for cross-type arguments; `string_to_it` throws a parse exception on malformed input.

---

## 6. Examples (C++ snippets)

```cpp
#include "googology/notations/ordinal/veblen/weakveblen/WeakVeblen.hpp"
#include <iostream>
using namespace googology;
using namespace googology::ordinal;

// Example 1: construct and expand (no evaluation)
WeakVeblen a("(2@3, 0@1)");
std::cout << a.to_string() << "\n";       // render (a@b) expression
a.expand(1);
std::cout << a.to_string() << "\n";       // rewritten form after one expansion

// Example 2: fundamental-sequence index (original unchanged)
WeakVeblen b("(0@1)");
WeakVeblen b1 = b[1];                      // 1st fundamental-sequence term; b unchanged
std::cout << b1.to_string() << "\n";

// Example 3: same-type compare; cross-type throws NotComparable
WeakVeblen c("(1@2)");
WeakVeblen d("(1@3)");
int r = c.compare(d);                      // lexicographic (primary key @b)
// c.compare(somePrss);   // throws NotComparable
```

---

## 7. Design notes

- **A notation is an expression; it is never evaluated**: `to_string()` only prints LaTeX/symbols; no `Evaluate` / `ToOrdinal`.
- **Derives from `Notation`, not `OrdinalNotation`**: weak-Veblen entries are ordinal pairs `(a@b)` and the article defines no standard form, so there are **no** `normalize` / `is_standard` / `isSuccessor` / `limit` / `roots`. This differs from PrSS / ε_pSS / ε_ωSS, which are natural-number **sequence** notations inheriting `OrdinalNotation`.
- **Faithful implementation (no added interpretation)**: expansion follows the article's six cases verbatim; ordinal arithmetic is supplied by `core::Ordinal`. `compare` is declared because the article §1.3 gives it; `expand_to` is undeclared and throws `UnsupportedOperation`.
- `compare` is a well-defined ordinal order (on standard-form expressions); cross-type throws `NotComparable`.

> Footnote: all function signatures are taken from the header `include/googology/notations/ordinal/veblen/weakveblen/WeakVeblen.hpp`.
