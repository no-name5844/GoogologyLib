# GoogologyLib (C++ branch)

A multi-language library that **collects notations** and provides their
**expansion** and (where well-defined) **comparison**. Each language lives on
its own git branch; `master` holds language-neutral `spec/` and
`tests/vectors/`. See `doc/design.md` for the full contract.

## Key principle: 序数记号 ≠ 大数记号

- **Ordinal notations** (Cantor, Veblen, Buchholz, Y, PPS, …): `compare` is
  well-defined (ordinal order).
- **Large-number notations** (Knuth ↑, Conway →, BEAF, …): `compare` is
  **generally UNDEFINED** and throws `NotComparable`. They still support
  `parse` / `serialize` / `expand` / `evaluate`.

Every notation declares which operations it supports via a **capability
bitset**, so the interface is always complete even when only some operations
are implemented.

## This branch implements (auditable C++)

| notation | file | ops |
|---|---|---|
| Knuth up-arrow (高德纳箭头) | `include/googology/notations/number/knuth/` | Parse, Serialize, Expand, Evaluate |
| Conway chained arrow (康威链式箭头) | `include/googology/notations/number/conway/` | Parse, Serialize, Expand, Evaluate |

Other notations are future slots (see `doc/design.md` taxonomy).

## Build

```bash
cmake -S . -B build
cmake --build build
./build/demo
ctest --test-dir build
```

Default integer type is `int64_t` (no external deps). For values beyond 2^63,
enable Boost's `cpp_int` (see the commented block in `CMakeLists.txt`).

## Quick example

```cpp
#include "googology/notations/number/knuth/Knuth.hpp"
using namespace googology::number;
Knuth k("2 ^^ 3");
std::cout << k.evaluate();   // 16
k.expand(1);                 // one rewrite step
// k.compare(other);         // throws NotComparable (undefined for large numbers)
```
