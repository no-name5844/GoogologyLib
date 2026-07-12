#pragma once
#include <string>
#include <vector>
#include "googology/core/OrdinalNotation.hpp"
#include "googology/core/Capability.hpp"

namespace googology {
namespace ordinal {

// EpspSS (ε_pSS) — 阶差型 (difference) natural-number sequence notation.
// Implements EXACTLY the definition in `study/notations/
// "epsilon_nSS & epsilon_omegaSS.md"` (section 1). No added interpretation:
//   * expansion follows the article's expandLen / expand verbatim
//     (case 2 uses the (m+n-L)th expandLen(A,k) index, case 3/4 use
//      (n+m-1)th expandLen(A,k); the indexed element is the RUNNING
//      sequence at stage k, not the original A)
//   * nothing is reinterpreted: an out-of-range index in the article's
//     formulas is left as written (a defensive bounds check throws rather
//     than silently wrapping — this is a guard, not a closure)
//   * isSuccessor() follows the article's successor clause (a sequence
//     ending in 1 is a successor). normalize() computes the standard form
//     (标准型) via the project's UNIVERSAL standard-form definition — an
//     expression is a legal expression iff it is obtainable from a limit
//     expression by finitely many expansions + taking a prefix. The user has
//     specified the limit expression for ε_pSS as (1, n), so the canonical
//     starter is (1, a_2). compare() is NOT in the article, but the user
//     specifies that under standard form the notation admits lexicographic
//     ordinal comparison (consistent with Prss). It is implemented as
//     lexicographic order over the sequence — valid when both operands are
//     in standard form; cross-type comparison throws NotComparable. The C++ branch is the
//     reference implementation; per project convention it must later be
//     ported to C/Java/Python/Lean4.
class EpspSS : public OrdinalNotation {
    BigInt p_ = 1;   // parameter p (cap on added amount), from the article

    // rightmost 1-based column index whose value is < a_n (the last element);
    // returns 0 if none (only possible when a_n == 0, i.e. all-zero seq).
    BigInt rightmostLess_(BigInt an) const;
    // expandLen(A, M): decrement last, then append M elements per the
    // article's case rules. Mutates *this.
    EpspSS& expandLen_(BigInt M);

public:
    EpspSS() { baseVal_ = 1; }
    explicit EpspSS(BigInt p) : p_(p) { baseVal_ = 1; }
    explicit EpspSS(const std::string& s) { string_to_it(s); baseVal_ = 1; }
    EpspSS(BigInt p, const std::string& s) : p_(p) { string_to_it(s); baseVal_ = 1; }

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
    EpspSS& expand_to(BigInt len) override;   // -> expandLen(A, len)

    // A[n] — the n-th term of the fundamental sequence of the ordinal A
    // denotes. Equivalent to expand(A, n). Does NOT mutate *this (returns a
    // copy), so repeated indexing A[1], A[2], ... is safe.
    EpspSS operator[](BigInt n) const;

    // Lexicographic ordinal comparison, valid when both operands are in
    // standard form (user-specified property, consistent with Prss).
    // Cross-type arguments throw NotComparable.
    int compare(const Notation& other) const override;

    friend std::istream& operator>>(std::istream& is, EpspSS& p);
};

} // namespace ordinal
} // namespace googology
