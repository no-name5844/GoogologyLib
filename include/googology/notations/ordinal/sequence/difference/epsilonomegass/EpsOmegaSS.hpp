#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "googology/core/Notation.hpp"
#include "googology/core/Capability.hpp"

namespace googology {
namespace ordinal {

// EpsOmegaSS (ε_ωSS) — 阶差型 (difference) natural-number sequence
// notation, the ω-limit of EpspSS (no cap on the added amount).
// See spec/notations/epsilon_omega_ss.md.
//
// A sequence A = (a_1,...,a_n) satisfies: a_i in N, a_1 = 0. (Same
// relaxed constraints as EpspSS — arbitrary jumps allowed.) Unlike EpspSS
// there is NO parameter p and NO case 4: the gap q = a_n - a_k is added
// in full (unbounded).
//
// expandLen(A, m):
//   m=0                     -> (a_1..a_n - 1)
//   m=k+1, a_n = a_k + 1  -> append (n+m-1 th A)        [add 0]
//   m=k+1, a_n = a_k + q  -> append (n+m-1 th A) + q   [add q, unbounded]
// where k = br = rightmost column index < a_n, L = n - br (tail length),
// and out-of-range indices wrap cyclically within the tail (see spec, ⚠).
// As with EpspSS, when a_n = a_k+1 the tail length is forced to L=1, so
// the case-2 index (m+n-L) equals the unified (n+m-1).
//
// The library NEVER evaluates to a number/ordinal: expand()/expand_to()/
// normalize() rewrite the internal sequence and return *this (an EpsOmegaSS
// object). to_string() emits LaTeX-ready text. Standard form (标准型) is
// computed by normalize(), adapted from YSequence::checkStandardAndNonMaximum.
// compare() is lexicographic (YSequence::_compare).
class EpsOmegaSS : public Notation {
    std::vector<BigInt> seq_;

    BigInt rightmostLess_(BigInt an) const;
    EpsOmegaSS& expandLen_(BigInt M);

    static BigInt expandUntilLarger_(EpsOmegaSS& work, const std::vector<BigInt>& target,
                                     size_t startIdx);

public:
    EpsOmegaSS() = default;
    explicit EpsOmegaSS(const std::string& s) { string_to_it(s); }

    std::string name() const override { return "epsilon_omega_ss"; }
    Family family() const override { return Family::Ordinal; }
    std::string subfamily() const override { return "sequence"; }
    std::string style() const override { return "difference"; }

    Capabilities capabilities() const override;

    void string_to_it(const std::string& s) override;
    std::string to_string() const override;

    EpsOmegaSS& expand(BigInt n) override;
    EpsOmegaSS& expand_to(BigInt len) override;

    void normalize() override;            // 标准型 (⚠ canonical starter to verify)
    bool isSuccessor() const override;

    int compare(const Notation& other) const override; // lexicographic; throws NotComparable

    friend std::istream& operator>>(std::istream& is, EpsOmegaSS& p);
};

} // namespace ordinal
} // namespace googology
