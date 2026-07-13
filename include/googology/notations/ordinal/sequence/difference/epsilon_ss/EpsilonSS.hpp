#pragma once
#include <string>
#include <vector>
#include "googology/core/OrdinalNotation.hpp"
#include "googology/core/Capability.hpp"

namespace googology {
namespace ordinal {

// ============================================================================
// ε_pSS / ε_ωSS — merged module (SAME type, written together)
//
// Project convention: notations that appear in the SAME source article are the
// SAME type and are written in ONE module. Both ε_pSS and ε_ωSS come from the
// single article `study/notations/"epsilon_nSS & epsilon_omegaSS.md"`
// (section 1 = ε_pSS, section 2 = ε_ωSS), so they live here together.
//
// Both are 阶差型 (difference) natural-number sequence notations, inherit
// `OrdinalNotation`, share `baseVal_ = 1`, and share an identical
// `expandLen_` / `compare` / `normalize` algorithm. The ONLY difference is
// the per-step addition:
//   * EpspSS    has a parameter p (a cap): case 4 caps the added amount at p.
//   * EpsOmegaSS has no p and no case 4: the gap q is added in full
//     (unbounded) — it is the ω-limit of EpspSS.
//
// Faithful policy (no added interpretation): the article's formulas are
// implemented VERBATIM; an out-of-range index is left as written and a
// defensive guard throws rather than silently wrapping. isSuccessor() follows
// the article's successor clause (a sequence ending in 1 is a successor).
// normalize() computes the standard form (标准型) via the project's UNIVERSAL
// standard-form definition (an expression is a legal expression iff obtainable
// from a limit expression by finitely many expansions + taking a prefix); the
// user-specified limit expression for both is (1, n), so the canonical
// starter is (1, a_2). compare() is NOT in the article, but the user
// specifies that under standard form the notation admits lexicographic ordinal
// comparison (consistent with Prss); it is implemented as lexicographic order
// over the sequence — valid when both operands are in standard form;
// cross-type comparison throws NotComparable. The C++ branch is the reference
// implementation; per project convention it must later be ported to
// C / Java / Python / Lean4.
// ============================================================================

// --- EpspSS (ε_pSS) ---
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

    // 创造者 / creator: zahin —— ε_pSS 由 zahin 命名（见 spec
    // epsilon_ss.zh.md / .en.md 的 Creator 元数据）。版本 / version: 1（基础版本）。
    std::string creator() const override { return "zahin"; }
    std::string version() const override { return "1"; }

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

// --- EpsOmegaSS (ε_ωSS) ---
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
