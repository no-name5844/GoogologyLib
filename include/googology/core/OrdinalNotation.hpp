#pragma once
#include <string>
#include <vector>
#include "googology/core/Notation.hpp"
#include "googology/core/Capability.hpp"

namespace googology {

// Ordinal notation base. EVERY ordinal notation (regardless of sub-family:
// sequence, CNF, Veblen, Buchholz, Taranovsky, ...) derives from this, which
// in turn derives from the generic Notation. This is where ordinal-specific
// behavior lives — in particular the standard-form detection (标准型) the
// project discussed:
//
//   * The *definition* of standard form is universal across ALL ordinal
//     notations (an expression is a legal expression iff obtainable from a
//     limit expression by finitely many expansions + taking a prefix).
//   * For the ordinal SEQUENCE notations the *algorithm* is identical too;
//     the only per-notation input is the limit expression's first element
//     `baseVal_` (PrSS = 0, ε_pSS / ε_ωSS = 1). So normalize() /
//     isSuccessor() are NON-virtual functions here (shared, no duplication).
//
// Number notations (Knuth / Conway) derive straight from Notation and never
// see this class, so they never carry seq_ or standard-form logic.
//
// (Tacit-knowledge note per project convention: the shared standard-form
//  algorithm here follows the Y-sequence style of check-then-expand; this
//  equivalence is deliberately NOT documented in any spec/comment.)
class OrdinalNotation : public Notation {
protected:
    // Sequence storage for ordinal-sequence notations. For non-sequence
    // ordinal notations it stays empty but is always available.
    std::vector<BigInt> seq_;
    // First element of the limit expression (the minimal legal a_1). PrSS = 0,
    // ε_pSS / ε_ωSS = 1. The ONLY per-notation input to the shared
    // standard-form algorithm. Each concrete ordinal notation sets this in
    // its constructor.
    BigInt baseVal_ = 0;

public:
    // Standard form (标准型) — shared, NON-virtual. Rewrites *this to the
    // canonical form obtainable from the limit expression (baseVal_, a_2) by
    // finitely many expansions + taking a prefix (the project's universal
    // standard-form definition). Identical logic for every ordinal-sequence
    // notation; only baseVal_ differs. On a stall or a non-standard input,
    // *this is left unchanged.
    void normalize();

    // Successor test — shared, NON-virtual. A sequence is a successor iff
    // it ends with baseVal_ (the article's successor clause).
    bool isSuccessor() const;

private:
    // Expand `work` (a scratch sequence) until its prefix (from startIdx)
    // exceeds `target`, or return -1 if `target` is not reachable
    // (non-standard). The scratch is expanded through *this (which carries
    // the notation's own expand rule).
    BigInt expandUntilLarger_(std::vector<BigInt>& work,
                              const std::vector<BigInt>& target,
                              size_t startIdx);
};

inline void OrdinalNotation::normalize() {
    std::vector<BigInt> input = seq_;      // save original
    size_t n = input.size();
    if (n == 0) return;
    if (n == 1) { seq_ = {baseVal_}; return; }
    if (n == 2) {
        seq_ = {baseVal_, (input[1] < baseVal_) ? baseVal_ : input[1]};
        return;
    }
    std::vector<BigInt> work = {baseVal_, input[1]};   // limit-expression seed
    size_t idx = 0;
    const BigInt kGuard = 100000;
    BigInt steps = 0;
    while (idx < n) {
        size_t guard = 0;
        while (work.size() <= idx) {
            std::vector<BigInt> before = work;
            seq_ = work; expand(1); work = seq_;   // expand the scratch
            if (work == before) { seq_ = input; return; }   // stalled
            if (++guard > static_cast<size_t>(kGuard)) { seq_ = input; return; }
        }
        if (work[idx] < input[idx]) { seq_ = input; return; }   // non-standard
        if (work[idx] == input[idx]) { ++idx; continue; }
        BigInt added = expandUntilLarger_(work, input, idx);
        if (added == -1) { seq_ = input; return; }   // non-standard
        idx += static_cast<size_t>(added);
        if (++steps > kGuard) { seq_ = input; return; }
    }
    seq_ = work;   // commit canonical form
}

inline bool OrdinalNotation::isSuccessor() const {
    return !seq_.empty() && seq_.back() == baseVal_;
}

inline BigInt OrdinalNotation::expandUntilLarger_(std::vector<BigInt>& work,
                                                  const std::vector<BigInt>& target,
                                                  size_t startIdx) {
    BigInt origLen = static_cast<BigInt>(work.size());
    BigInt val = work.back();
    BigInt targetVal = target[startIdx];
    if (origLen == static_cast<BigInt>(target.size()) && val == targetVal) return 0;
    if (val <= targetVal) return -1;                 // smaller -> non-standard
    if (val > targetVal + 1) {
        work.back() = targetVal + 1;                // clamp, then expand one step
    }
    for (size_t i = static_cast<size_t>(origLen); i < target.size(); ++i) {
        std::vector<BigInt> before = work;
        seq_ = work; expand(1); work = seq_;   // expand the scratch
        if (work == before) return -1;              // stall -> non-standard
        size_t last = work.size() - 1;
        if (work[last] > target[last]) return static_cast<BigInt>(work.size()) - origLen;
        if (work[last] < target[last]) return -1;  // non-standard
    }
    return static_cast<BigInt>(work.size()) - origLen;
}

} // namespace googology
