#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "googology/core/Notation.hpp"
#include "googology/core/Capability.hpp"

namespace googology {
namespace ordinal {

// EpspSS (ε_pSS) — 阶差型 (difference) natural-number sequence notation,
// a parameterized generalization of PrSS. See spec/notations/epsilon_p_ss.md.
//
// A sequence A = (a_1,...,a_n) satisfies: a_i in N, a_1 = 0. (The
// PrSS extra constraints Δ<=1 and plateau-rule are RELAXED, so arbitrary
// jumps are allowed.) Parameter p in N caps how much is added per expansion
// step (case 4 below).
//
// expandLen(A, m):
//   m=0                                   -> (a_1..a_n - 1)
//   m=k+1, a_n = a_k + 1                    -> append (n+m-1 th A)        [add 0]
//   m=k+1, a_n = a_k + q (1<q<=p)         -> append (n+m-1 th A) + q   [add q]
//   m=k+1, a_n - a_k > p                   -> append (n+m-1 th A) + p   [add p, capped]
// where k = br = rightmost column index < a_n, L = n - br (tail length),
// and out-of-range indices wrap cyclically within the tail (see spec, ⚠).
// Note: when a_n = a_k+1 (gap 1) the tail length is forced to L=1, so
// the case-2 index (m+n-L) coincides with the unified (n+m-1); we use the
// unified form for all cases and only vary the added amount.
//
// The library NEVER evaluates to a number/ordinal: expand()/expand_to()/
// normalize() rewrite the internal sequence and return *this (an EpspSS
// object). to_string() emits LaTeX-ready text. Standard form (标准型) is
// computed by normalize(), adapted from YSequence::checkStandardAndNonMaximum
// using EpspSS's own expansion. compare() is lexicographic (YSequence::_compare).
class EpspSS : public Notation {
    std::vector<BigInt> seq_;
    BigInt p_ = 1;   // parameter p (cap on added amount)

    // rightmost 1-based column index whose value is < an (last element);
    // returns 0 if none (tail = whole sequence).
    BigInt rightmostLess_(BigInt an) const;
    // expandLen(A, M): decrement last, then append M elements from the tail
    // (columns br+1..n) cyclically, adding 0 / q / p per the gap mode.
    // Mutates *this.
    EpspSS& expandLen_(BigInt M);

    // Expand `work` until its sequence exceeds `target` from startIdx; returns
    // the number of elements added, or -1 if non-standard. (YSequence::_expandUntilLarger)
    static BigInt expandUntilLarger_(EpspSS& work, const std::vector<BigInt>& target,
                                     size_t startIdx);

public:
    EpspSS() = default;
    explicit EpspSS(BigInt p) : p_(p) {}
    explicit EpspSS(const std::string& s) { string_to_it(s); }
    EpspSS(BigInt p, const std::string& s) : p_(p) { string_to_it(s); }

    void set_p(BigInt p) { p_ = p; }
    BigInt p() const { return p_; }

    std::string name() const override { return "epsilon_p_ss"; }
    Family family() const override { return Family::Ordinal; }
    std::string subfamily() const override { return "sequence"; }
    std::string style() const override { return "difference"; }

    Capabilities capabilities() const override;

    void string_to_it(const std::string& s) override;
    std::string to_string() const override;

    // These return the notation's OWN type (a rewritten EpspSS), not a string.
    EpspSS& expand(BigInt n) override;
    EpspSS& expand_to(BigInt len) override;

    void normalize() override;            // 标准型 (⚠ canonical starter to verify)
    bool isSuccessor() const override;

    int compare(const Notation& other) const override; // lexicographic; throws NotComparable

    friend std::istream& operator>>(std::istream& is, EpspSS& p);
};

} // namespace ordinal
} // namespace googology
