#pragma once
#include <string>
#include <vector>
#include "googology/core/Notation.hpp"
#include "googology/core/Capability.hpp"

namespace googology {
namespace ordinal {

// EpsOmegaSS (ε_ωSS) — 阶差型 (difference) natural-number sequence
// notation, the ω-limit of EpspSS (no cap on the added amount).
// Implements EXACTLY the definition in `study/notations/
// "epsilon_nSS & epsilon_omegaSS.md"` (section 2). Same faithful policy
// as EpspSS: the article's formulas verbatim, no closure; normalize /
// compare / isSuccessor are not in the article, so they are left to the
// base class (they throw). No parameter p and no case 4: the gap
// q = a_n - a_br is added in full (unbounded).
class EpsOmegaSS : public Notation {
    std::vector<BigInt> seq_;

    BigInt rightmostLess_(BigInt an) const;
    EpsOmegaSS& expandLen_(BigInt M);

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
    EpsOmegaSS& expand_to(BigInt len) override;   // -> expandLen(A, len)

    friend std::istream& operator>>(std::istream& is, EpsOmegaSS& p);
};

} // namespace ordinal
} // namespace googology
