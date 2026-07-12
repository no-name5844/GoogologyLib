#pragma once
#include <string>
#include <vector>
#include "googology/core/OrdinalNotation.hpp"
#include "googology/core/Capability.hpp"

namespace googology {
namespace ordinal {

// EpsOmegaSS (ε_ωSS) — 阶差型 (difference) natural-number sequence
// notation, the ω-limit of EpspSS (no cap on the added amount).
// Implements EXACTLY the definition in `study/notations/
// "epsilon_nSS & epsilon_omegaSS.md"` (section 2). Same faithful policy
// as EpspSS: the article's formulas verbatim, no closure. isSuccessor()
// follows the article's successor clause (ends in 1); normalize() computes
// the standard form (标准型) via the project's UNIVERSAL standard-form
// definition with the user-specified limit expression (1, n) (starter
// (1, a_2)). compare() is not in the article, but the user specifies
// that under standard form the notation admits lexicographic ordinal
// comparison (consistent with Prss). It is implemented as lexicographic
// order over the sequence — valid when both operands are in standard
// form; cross-type comparison throws NotComparable. No parameter p and
// no case 4: the gap
// q = a_n - a_br is added in full (unbounded).
class EpsOmegaSS : public OrdinalNotation {
    BigInt rightmostLess_(BigInt an) const;
    EpsOmegaSS& expandLen_(BigInt M);

public:
    EpsOmegaSS() { baseVal_ = 1; }
    explicit EpsOmegaSS(const std::string& s) { string_to_it(s); baseVal_ = 1; }

    std::string name() const override { return "epsilon_omega_ss"; }
    Family family() const override { return Family::Ordinal; }
    std::string subfamily() const override { return "sequence"; }
    std::string style() const override { return "difference"; }

    Capabilities capabilities() const override;

    void string_to_it(const std::string& s) override;
    std::string to_string() const override;

    EpsOmegaSS& expand(BigInt n) override;
    EpsOmegaSS& expand_to(BigInt len) override;   // -> expandLen(A, len)

    // A[n] — the n-th term of the fundamental sequence of the ordinal A
    // denotes. Equivalent to expand(A, n). Does NOT mutate *this.
    EpsOmegaSS operator[](BigInt n) const;

    // Lexicographic ordinal comparison, valid when both operands are in
    // standard form (user-specified property, consistent with Prss).
    // Cross-type arguments throw NotComparable.
    int compare(const Notation& other) const override;

    friend std::istream& operator>>(std::istream& is, EpsOmegaSS& p);
};

} // namespace ordinal
} // namespace googology
