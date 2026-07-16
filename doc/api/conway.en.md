# Conway chained-arrow notation

> One-line definition: a large-number notation using `→` chains for iterated hyper-exponentiation (rules `[a->b]=a^b`, `X->1->Y=X`, `X->a->b = X->(X->a-1->b)->b-1`). This library performs **only symbolic expansion and LaTeX serialization — it never evaluates**.

Header: `include/googology/notations/number/conway/Conway.hpp`
Namespace: `googology::number`

---

## 1. Taxonomy

| Interface | Returns |
|-----------|---------|
| `name()` | `"conway"` |
| `family()` | `Family::Number` |
| `subfamily()` | `""` |
| `style()` | `""` |

**Category**: **large-number notation (number)**. Conway derives straight from `Notation` and carries **no** ordinal standard-form logic.

---

## 2. Creator / Version

| Interface | Returns |
|-----------|---------|
| `creator()` | `"John Conway"` |
| `version()` | `"1"` |

---

## 3. Capabilities

`capabilities()` declares support for:

- `Op::FromString` ✔
- `Op::ToString` ✔
- `Op::Expand` ✔
- `Op::ExpandTo` ✔
- `Op::Compare` ✘ (comparison of large-number notations is undefined)

`Compare` is not supported: calling `compare` throws `NotComparable`. No `Normalize` / `Successor`.

---

## 4. Inherited base interface (`Notation`)

Identical to Knuth (see Knuth doc §4): `name` / `family` / `subfamily` / `style` / `creator` / `version` / `capabilities` / `can` / `string_to_it` / `to_string` / `compare` (base throws `NotComparable`) / `expand` / `expand_to` / `comparable` / `print` / `operator<<` / `reduce`.

> Again: operations return the notation's **own object**, never a string; there is no numeric-evaluation interface.

---

## 5. This notation's own API (core)

### 5.1 Internal node struct `CNode` (public nested struct)

```cpp
struct CNode {
    bool isInt = true;                    // true = integer leaf; false = nested subchain
    BigInt val = 0;                       // integer value
    std::vector<CNode> sub;               // subchain elements

    static CNode value(BigInt v);                       // build an integer leaf
    static CNode subchain(std::vector<CNode> s);        // build a nested subchain
};
```

### 5.2 Construction & parsing

```cpp
Conway() = default;
explicit Conway(const std::string& s);    // equivalent to default-construct then string_to_it(s)
```

### 5.3 Overridden interface

```cpp
std::string name() const override;        // "conway"
Family      family() const override;      // Family::Number
std::string creator() const override;     // "John Conway"
std::string version() const override;     // "1"

Capabilities capabilities() const override;

void        string_to_it(const std::string& s) override;
std::string to_string() const override;            // emit LaTeX, e.g. "3 \rightarrow 3 \rightarrow 2"

Conway& expand(BigInt n) override;        // in-place rewrite chain n steps (rules 2/3), returns *this
Conway& expand_to(BigInt len) override;   // expand to length len, returns *this
```

### 5.4 Stream input

```cpp
friend std::istream& operator>>(std::istream& is, Conway& c);
```

**Behavior notes**
- `expand` / `expand_to`: modify `*this` **in place** and return a self reference. A single `stepOnce` applies rules 2/3 at the outermost applicable position; if the top level has no rule but a nested sub-chain does, the step is applied inside that sub-chain (`recurseSub` drives full reduction of nested chains).
- `to_string()`: only prints LaTeX, never evaluates.
- Exceptions: `compare` throws `NotComparable`; malformed input throws `std::invalid_argument` (from `string_to_it`).

---

## 6. Examples (C++ snippets)

```cpp
#include "googology/notations/number/conway/Conway.hpp"
#include <iostream>
using namespace googology;
using namespace googology::number;

// Example 1: construct and expand (no evaluation)
Conway c("3 -> 3 -> 2");
std::cout << c.to_string() << "\n";      // LaTeX: 3 \rightarrow 3 \rightarrow 2
c.expand(1);
std::cout << c.to_string() << "\n";      // rewritten form after one expansion

// Example 2: expand_to a target length
Conway c2("4 -> 4 -> 4");
c2.expand_to(2);
std::cout << c2 << "\n";

// Example 3: capability query
Conway c3("2 -> 3");
std::cout << std::boolalpha << c3.can(Op::Expand) << "\n";   // true
std::cout << c3.can(Op::Compare) << "\n";                     // false
```

---

## 7. Design notes

- **A notation is an expression; it is never evaluated**: `to_string()` only prints LaTeX; no `Evaluate` / `ToOrdinal`.
- **Large-number notation ≠ ordinal notation**: Conway represents integers/functions whose magnitude is not comparable in the library; comparing within or across families throws `NotComparable`.
- `expand` / `expand_to` are in-place rewrites returning `*this`; copy first to preserve the original (default copy deep-copies the `std::vector<CNode>`).
- Expansion rules (`spec/notations/conway.md`): `[a->b]=a^b`, `X->1->Y=X`, `X->a->b = X->(X->a-1->b)->b-1`.

> Footnote: all function signatures in this document are taken from the header `include/googology/notations/number/conway/Conway.hpp`.
