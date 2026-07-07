#pragma once
#include <cstdint>

namespace googology {

// Operations a notation may support. This is the capability model: every
// notation declares which operations it implements. Operations not declared
// are still part of the interface (so "other operations are included"), but
// calling them throws UnsupportedOperation. See doc/design.md.
//
// NOTE: there is NO `Evaluate` capability. The library never computes a
// numeric value for a notation; "how to compute" is provided by expand() /
// expand_to() which return the notation's own symbolic (LaTeX) form.
enum class Op : uint8_t {
    FromString = 0,
    ToString = 1,
    Normalize = 2,
    Compare = 3,
    Expand = 4,
    ExpandTo = 5,
    Successor = 6
};

class Capabilities {
    uint8_t bits_ = 0;
public:
    void set(Op o) { bits_ |= (1u << static_cast<uint8_t>(o)); }
    bool has(Op o) const { return (bits_ & (1u << static_cast<uint8_t>(o))) != 0; }
    uint8_t raw() const { return bits_; }
};

} // namespace googology
