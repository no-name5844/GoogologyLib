#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "googology/core/Notation.hpp"
#include "googology/core/Capability.hpp"

namespace googology {
namespace ordinal {

// PrSS (Primitive Sequence System) — 阶差型 (difference) natural-number
// sequence notation. See spec/notations/prss.md.
//
// A sequence A = (a_1,...,a_n) satisfies: a_i in N, a_{i+1}-a_i <= 1, a_1 = 0,
// and a plateau is followed by a non-increase. The library NEVER evaluates to a
// number/ordinal: expand()/expand_to()/normalize() rewrite the internal
// sequence and return *this (a Prss object). to_string() emits LaTeX.
//
// Standard form (标准型) is computed by normalize(), adapted from
// YSequence::checkStandardAndNonMaximum (YSequence.cpp), using PrSS's own
// expansion. The comparison used by compare() is lexicographic, mirroring
// YSequence::_compare.
class Prss : public Notation {
    std::vector<BigInt> seq_;

    // rightmost 1-based column index whose value is < an (the last element);
    // returns 0 if none (tail = whole sequence).
    BigInt rightmostLess_(BigInt an) const;
    // expandLen(A, M): decrement last, then append M elements from the tail
    // (columns br+1..n) cyclically. Mutates *this.
    Prss& expandLen_(BigInt M);

    // Expand `work` until its sequence exceeds `target` in the prefix starting
    // at startIdx; returns the number of elements added, or -1 if non-standard.
    // Mirrors YSequence::_expandUntilLarger. ⚠ exact PrSS semantics to verify.
    static BigInt expandUntilLarger_(Prss& work, const std::vector<BigInt>& target,
                                      size_t startIdx);

public:
    Prss() = default;
    explicit Prss(const std::string& s) { string_to_it(s); }

    std::string name() const override { return "prss"; }
    Family family() const override { return Family::Ordinal; }
    std::string subfamily() const override { return "sequence"; }
    std::string style() const override { return "difference"; }

    Capabilities capabilities() const override;

    void string_to_it(const std::string& s) override;
    std::string to_string() const override;

    // These return the notation's OWN type (a rewritten Prss), not a string.
    Prss& expand(BigInt n) override;
    Prss& expand_to(BigInt len) override;

    // Standard form (标准型). ⚠ canonical starter + exact semantics for PrSS
    // need verification (adapted from YSequence.cpp).
    void normalize() override;
    bool isSuccessor() const override;

    // Lexicographic comparison (from YSequence::_compare). Throws NotComparable
    // for non-Prss arguments.
    int compare(const Notation& other) const override;

    friend std::istream& operator>>(std::istream& is, Prss& p);
};

} // namespace ordinal
} // namespace googology
