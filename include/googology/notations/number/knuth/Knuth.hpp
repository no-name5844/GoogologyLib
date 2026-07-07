#pragma once
#include <string>
#include <iostream>
#include <memory>
#include "googology/core/Notation.hpp"
#include "googology/core/Capability.hpp"

namespace googology {
namespace number {

// One node of a Knuth up-arrow expression.
//   * a leaf is a constant integer value;
//   * an inner node is `baseVal ↑^height exp` (exp may be a subtree).
// In Knuth's form the left operand is always a constant, so only the exponent
// needs to be a (recursive) subtree. This lets an expanded form such as
// `a ↑^{c-1} (a ↑^c (b-1))` be stored back inside the SAME class instead of
// being flattened to a string.
struct KNode {
    bool isVal = true;
    BigInt val = 0;                       // leaf value
    BigInt baseVal = 0;                   // arrow: left operand (a constant)
    BigInt height = 1;                    // arrow: height
    std::unique_ptr<KNode> exp;           // arrow: right operand (subtree)
    static KNode value(BigInt v) { KNode n; n.isVal = true; n.val = v; return n; }
    static KNode arrow(BigInt base, BigInt h, KNode e) {
        KNode n; n.isVal = false; n.baseVal = base; n.height = h;
        n.exp = std::make_unique<KNode>(std::move(e)); return n;
    }
};

// Knuth up-arrow notation (高德纳箭头).
//   a ^c b = a ^ b                 (c = 1)
//   a ^c b = a                     (b = 1)
//   a ^c b = a ^(c-1) (a ^c (b-1)) (c > 1, b > 1)
//
// Output is LaTeX (e.g. `2 \uparrow\uparrow 3`). The library never computes a
// numeric value: expand() / expand_to() / reduce() rewrite the internal AST and
// return *this (a Knuth object); to_string() turns it into LaTeX on demand.
class Knuth : public Notation {
    KNode root_;

    // One single rewrite step on the AST (never a string).
    static KNode step(const KNode& node);
    // Deep clone (KNode holds a unique_ptr so it is non-copyable).
    static KNode cloneNode(const KNode& node);
    // LaTeX (no numeric evaluation) of the AST.
    static std::string tex(const KNode& node);

public:
    Knuth() = default;
    explicit Knuth(const std::string& s) { string_to_it(s); }

    std::string name() const override { return "knuth"; }
    Family family() const override { return Family::Number; }

    // Supports FromString / ToString / Expand / ExpandTo. Compare is NOT
    // supported: large-number comparison is undefined.
    Capabilities capabilities() const override;

    void string_to_it(const std::string& s) override;
    std::string to_string() const override;

    // These return the notation's OWN type (a rewritten Knuth), not a string.
    Knuth& expand(BigInt n) override;
    Knuth& expand_to(BigInt len) override;

    friend std::istream& operator>>(std::istream& is, Knuth& k);
};

} // namespace number
} // namespace googology
