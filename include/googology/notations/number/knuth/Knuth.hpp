#pragma once
#include <string>
#include <iostream>
#include "googology/core/Notation.hpp"
#include "googology/core/Capability.hpp"

namespace googology {
namespace number {

// Knuth up-arrow notation (高德纳箭头).
//   a ^c b = a ^ b                 (c = 1)
//   a ^c b = a                     (b = 1)
//   a ^c b = a ^(c-1) (a ^c (b-1)) (c > 1, b > 1)
//
// Output is LaTeX (e.g. `2 \uparrow\uparrow 3`). The library never computes a
// numeric value; expand() / reduce() return the symbolic rewrite in LaTeX.
class Knuth : public Notation {
    BigInt a_ = 0, c_ = 1, b_ = 0;

    // LaTeX of a ^c b after `depth` single-step rewrites (depth=0 -> literal).
    static std::string fmt(BigInt a, BigInt c, BigInt b, BigInt depth);

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
    std::string expand(BigInt n) const override;
    std::string expand_to(BigInt len) const override;

    friend std::istream& operator>>(std::istream& is, Knuth& k);
};

} // namespace number
} // namespace googology
