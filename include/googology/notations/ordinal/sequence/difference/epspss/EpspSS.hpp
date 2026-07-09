#pragma once
#include <string>
#include <vector>
#include "googology/core/Notation.hpp"
#include "googology/core/Capability.hpp"

namespace googology {
namespace ordinal {

// EpspSS (ε_pSS) — 阶差型 (difference) natural-number sequence notation.
// Implements EXACTLY the definition in `study/notations/
// "epsilon_nSS & epsilon_omegaSS.md"` (section 1). No added interpretation:
//   * expansion follows the article's expandLen / expand verbatim
//     (case 2 uses the (m+n-L)th A index, case 3/4 use (n+m-1)th A)
//   * nothing is reinterpreted: an out-of-range index in the article's
//     formulas is left as written (a defensive bounds check throws rather
//     than silently wrapping — this is a guard, not a closure)
//   * normalize / compare / isSuccessor are NOT defined by the article,
//     so they are left to the base class (they throw UnsupportedOperation /
//     NotComparable). The C++ branch is the reference implementation;
//     per project convention it must later be ported to C/Java/Python/Lean4.
class EpspSS : public Notation {
    std::vector<BigInt> seq_;
    BigInt p_ = 1;   // parameter p (cap on added amount), from the article

    // rightmost 1-based column index whose value is < a_n (the last element);
    // returns 0 if none (only possible when a_n == 0, i.e. all-zero seq).
    BigInt rightmostLess_(BigInt an) const;
    // expandLen(A, M): decrement last, then append M elements per the
    // article's case rules. Mutates *this.
    EpspSS& expandLen_(BigInt M);

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
    EpspSS& expand_to(BigInt len) override;   // -> expandLen(A, len)

    friend std::istream& operator>>(std::istream& is, EpspSS& p);
};

} // namespace ordinal
} // namespace googology
