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
// computes a numeric value: expand() / expand_to() rewrite the internal chain
// and return *this (a Conway object); to_string() turns it into LaTeX on demand.
class Conway : public Notation {
    std::vector<CNode> chain_;

    static std::string ser(const std::vector<CNode>& ch);
    // One single rewrite step (rules 2/3) at the outermost applicable position;
    // if the top level has no rule but a nested sub-chain does, the step is
    // applied inside that sub-chain. Linear in chain size; never computes a value.
    static std::vector<CNode> stepOnce(const std::vector<CNode>& ch);
    // Apply stepOnce to each immediate sub-chain; return ch unchanged if none
    // of them can step (used to drive full reduction of nested chains).
    static std::vector<CNode> recurseSub(const std::vector<CNode>& ch);

public:
    Conway() = default;
    explicit Conway(const std::string& s) { string_to_it(s); }

    std::string name() const override { return "conway"; }
    Family family() const override { return Family::Number; }

    // 创造者 / creator: John Conway（康威链式箭头，1960s）。版本 1。
    std::string creator() const override { return "John Conway"; }
    std::string version() const override { return "1"; }

    // Supports FromString / ToString / Expand / ExpandTo. Compare is NOT
    // supported: large-number comparison is undefined.
    Capabilities capabilities() const override;

    void string_to_it(const std::string& s) override;
    std::string to_string() const override;

    // These return the notation's OWN type (a rewritten Conway), not a string.
    Conway& expand(BigInt n) override;
    Conway& expand_to(BigInt len) override;

    friend std::istream& operator>>(std::istream& is, Conway& c);
};

} // namespace number
} // namespace googology
