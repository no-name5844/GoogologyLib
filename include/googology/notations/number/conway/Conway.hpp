#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "googology/core/Notation.hpp"
#include "googology/core/Capability.hpp"

namespace googology {
namespace number {

// A chain element: either an integer leaf, or a nested chain (parenthesized).
struct CNode {
    bool isInt = true;
    BigInt val = 0;
    std::vector<CNode> sub;
    static CNode value(BigInt v) { CNode n; n.isInt = true; n.val = v; return n; }
    static CNode subchain(std::vector<CNode> s) { CNode n; n.isInt = false; n.sub = std::move(s); return n; }
};

// Conway chained-arrow notation (康威链式箭头).
// Rules (per spec/notations/conway.md):
//   1. [a -> b]                = a ^ b
//   2. X -> 1 -> Y             = X
//   3. X -> a -> b             = X -> (X -> a-1 -> b) -> b-1
//
// Output is LaTeX (e.g. `3 \rightarrow 3 \rightarrow 2`). The library never
// computes a numeric value; expand() / reduce() return the symbolic rewrite.
class Conway : public Notation {
    std::vector<CNode> chain_;

    static std::string ser(const std::vector<CNode>& ch);
    // One single rewrite step (rules 2/3) at the outermost applicable position;
    // if the top level has no rule but a nested sub-chain does, the step is
    // applied one level inside that sub-chain. Linear in chain size.
    static std::vector<CNode> stepOnce(const std::vector<CNode>& ch);

public:
    Conway() = default;
    explicit Conway(const std::string& s) { string_to_it(s); }

    std::string name() const override { return "conway"; }
    Family family() const override { return Family::Number; }

    // Supports FromString / ToString / Expand / ExpandTo. Compare is NOT
    // supported: large-number comparison is undefined.
    Capabilities capabilities() const override;

    void string_to_it(const std::string& s) override;
    std::string to_string() const override;
    std::string expand(BigInt n) const override;
    std::string expand_to(BigInt len) const override;

    friend std::istream& operator>>(std::istream& is, Conway& c);
};

} // namespace number
} // namespace googology
