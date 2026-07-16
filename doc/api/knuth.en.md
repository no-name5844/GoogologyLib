# Knuth up-arrow notation

> One-line definition: a large-number notation using `↑` (up-arrows) to denote iterated hyper-exponentiation; higher arrow counts mean dramatically faster growth. This library performs **only symbolic expansion and LaTeX serialization — it never collapses an expression to a numeric value**.

Header: `include/googology/notations/number/knuth/Knuth.hpp`
Namespace: `googology::number`

---

## 1. Taxonomy

| Interface | Returns |
|-----------|---------|
| `name()` | `"knuth"` |
| `family()` | `Family::Number` |
| `subfamily()` | `""` (not overridden; base-class default empty) |
| `style()` | `""` (not overridden; base-class default empty) |

**Category**: **large-number notation (number)**. Knuth derives straight from the generic base `Notation` and carries **no** ordinal standard-form logic (`normalize` / `is_standard` / `isSuccessor` / `limit` / `roots` do not exist here).

---

## 2. Creator / Version

| Interface | Returns |
|-----------|---------|
| `creator()` | `"Donald Knuth"` |
| `version()` | `"1"` |

---

## 3. Capabilities

`capabilities()` declares the supported operations (query with `can(Op::X)`):

- `Op::FromString` ✔
- `Op::ToString` ✔
- `Op::Expand` ✔
- `Op::ExpandTo` ✔
- `Op::Compare` ✘ (comparison of large-number notations is **undefined** in the library)

**`Compare` is not supported**: large-number notations have no well-defined order. `Notation::compare()` throws `NotComparable` by default for any `Notation` argument; Knuth does not override `compare`, so calling it always throws `NotComparable`. There is also no `Op::Normalize` / `Op::Successor` — this notation has neither a standard form nor a successor concept.

---

## 4. Inherited base interface (`Notation`)

Knuth inherits directly from `googology::Notation`, so it exposes all of the following base members (some overridden by this notation):

- Taxonomy: `name()` / `family()` / `subfamily()` / `style()`
- Metadata: `creator()` / `version()`
- Capability: `Capabilities capabilities() const` / `bool can(Op op) const`
- Strings: `void string_to_it(const std::string&)` / `std::string to_string() const`
- Operations: `int compare(const Notation&) const` (base throws `NotComparable`) / `Notation& expand(BigInt)` / `Notation& expand_to(BigInt)`
- Utility: `bool comparable() const` (returns `can(Op::Compare)`, here `false`)
- Output: `std::ostream& print(std::ostream& = std::cout) const` / `friend std::ostream& operator<<(std::ostream&, const Notation&)`
- Reduce: `Notation& reduce()` — repeatedly `expand(1)` until the form stabilizes (max 1000 steps), returns `*this`

> Note: `expand` / `expand_to` / `reduce` return the notation's **own object** (so it can be expanded further or serialized) and **never** a string. This library has no `Evaluate` / `ToOrdinal` numeric-evaluation interface.

---

## 5. This notation's own API (core)

### 5.1 Internal node struct `KNode` (public nested struct)

```cpp
struct KNode {
    bool isVal = true;                                  // true = integer leaf; false = arrow node
    BigInt val = 0;                                     // leaf value
    BigInt baseVal = 0;                                 // arrow: left operand (constant a)
    BigInt height = 1;                                  // arrow: height c
    std::unique_ptr<KNode> exp;                         // arrow: right operand (subtree)

    static KNode value(BigInt v);                       // build a leaf node
    static KNode arrow(BigInt base, BigInt h, KNode e); // build an a ↑^h e node
};
```

In Knuth's form the left operand is always a constant, so only the exponent (right operand) needs to be a recursive subtree. An expanded form such as `a ↑^{c-1} (a ↑^c (b-1))` is stored back inside the **same** `KNode` type rather than being flattened to a string.

### 5.2 Construction & parsing

```cpp
Knuth() = default;                          // default-empty (no expression initialized)
explicit Knuth(const std::string& s);       // equivalent to default-construct then string_to_it(s)
```

### 5.3 Overridden interface

```cpp
std::string name() const override;          // returns "knuth"
Family      family() const override;        // returns Family::Number
std::string creator() const override;       // returns "Donald Knuth"
std::string version() const override;       // returns "1"

Capabilities capabilities() const override; // see §3

void        string_to_it(const std::string& s) override; // parse into internal AST
std::string to_string() const override;                  // emit LaTeX, e.g. "2 \uparrow\uparrow 3"

Knuth& expand(BigInt n) override;    // in-place rewrite the AST n steps, returns *this
Knuth& expand_to(BigInt len) override; // expand to length len (repeated expand), returns *this
```

### 5.4 Stream input

```cpp
friend std::istream& operator>>(std::istream& is, Knuth& k);  // calls string_to_it
```

**Behavior notes**
- `expand` / `expand_to`: modify `*this` **in place** and return a reference to self (chainable for further expansion).
- `to_string()`: only prints the LaTeX form — it **never** evaluates the expression to a number.
- Exceptions: `string_to_it` throws `std::invalid_argument` on malformed input (right-associative recursive-descent parser); `compare` throws `NotComparable`; `reduce` relies on `expand` / `to_string` internally and never throws numeric-evaluation errors.

---

## 6. Examples (C++ snippets)

```cpp
#include "googology/notations/number/knuth/Knuth.hpp"
#include <iostream>
using namespace googology;
using namespace googology::number;

// Example 1: construct and expand (no evaluation, just AST rewrite)
Knuth k("2 ^^ 3");                 // parsed from ASCII form; internally 2 ↑↑ 3
std::cout << k.to_string() << "\n"; // prints LaTeX: 2 \uparrow\uparrow 3
k.expand(1);
std::cout << k.to_string() << "\n"; // rewritten form after one expansion (still symbolic)

// Example 2: expand_to a target length
Knuth k2("3 ^^ 2 ^^ 2");
k2.expand_to(3);
std::cout << k2 << "\n";           // operator<< calls to_string()

// Example 3: capability query and cross-family comparison
Knuth k3("2 ^ 3");
if (!k3.can(Op::Compare))
    std::cout << "Knuth is not comparable\n"; // large-number notation has no comparison
// k3.compare(other);  // any call throws NotComparable
```

---

## 7. Design notes

- **A notation is an expression; it is never evaluated**: `to_string()` only prints LaTeX/symbols and never collapses to a number. There is no `Evaluate` / `ToOrdinal` numeric-evaluation interface (see `core/Capability.hpp`).
- **Large-number notation ≠ ordinal notation**: Knuth represents integers (or functions) whose magnitude is **not comparable** in the library — comparing within or across families throws `NotComparable`.
- `expand` / `expand_to` are **in-place rewrites**: they mutate `*this` and return a self reference; to keep the original, copy first (the default copy constructor deep-copies the `unique_ptr` subtree).
- Parse grammar (right-associative arrow tower): `expr := INT ( '^'+ expr )?`, so `a ^^ b ^^ c` parses as `a ↑^{2} (b ↑^{2} c)`.

> Footnote: all function signatures in this document are taken from the header `include/googology/notations/number/knuth/Knuth.hpp`.
