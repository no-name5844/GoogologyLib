#pragma once
#include <cstdint>

namespace googology {

// Operations a notation may support. This is the capability model: every
// notation declares which operations it implements. Operations not declared
// are still part of the interface (so "other operations are included"), but
// calling them throws UnsupportedOperation. See doc/design.md.
enum class Op : uint8_t {
    Parse = 0,
    Serialize = 1,
    Normalize = 2,
    Compare = 3,
    Expand = 4,
    ExpandTo = 5,
    Evaluate = 6,
    Successor = 7
};

class Capabilities {
    uint8_t bits_ = 0;
public:
    void set(Op o) { bits_ |= (1u << static_cast<uint8_t>(o)); }
    bool has(Op o) const { return (bits_ & (1u << static_cast<uint8_t>(o))) != 0; }
    uint8_t raw() const { return bits_; }
};

} // namespace googology
