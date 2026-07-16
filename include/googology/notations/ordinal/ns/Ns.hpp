#pragma once
#include <string>
#include <vector>
#include "googology/core/Notation.hpp"
#include "googology/core/Capability.hpp"
#include "googology/core/Ordinal.hpp"
#include "googology/core/Rational.hpp"

namespace googology {
namespace ordinal {

// ===========================================================================
// Ns / n,m-Ns — Nonlinear successo (by 送到本到 (UTF-8) & nnc)
//
// Source: study/notations/Ns.md. Ns is the (n,m)=(2,2) special case of
// the parameterized family n,m-Ns. The notation names the REAL x = α th NS,
// an element of an ordinal-indexed real sequence. We keep the expression
// symbolic (α th NS) and — per the user's "采用累加权重分数来表达" —
// expose the value as an ACCUMULATED WEIGHT FRACTION of the step terms
// 1/f(β), i.e. int + w1/d1 + w2/d2 + ... (e.g. "2 + 5/11 + 62/111"),
// where f(β) = n^β for finite β (so each weight is 1). Only FINITE α
// yields an exact Rational; limit indices need a fundamental sequence and are
// NOT auto-computed (see spec/notations/ns.*.md and design.md §11).
//
// The ordinal index α is stored as core::Ordinal; expand(k) delegates to
// α.expand(k) — the project's §11 pluggable fundamental-sequence source.
// compare() reduces to comparing the indices, since a_α is strictly
// increasing in α (each step adds a positive term 1/f(β)).
// ===========================================================================
class Ns : public Notation {
    Ordinal alpha_;          // ordinal index α (the "α th NS")
    BigInt n_ = 2;         // successor-step factor & NS-limit FS index
    BigInt m_ = 2;         // f-limit FS index

    // true iff alpha is a finite integer k; returns k via outK.
    static bool isFiniteInt_(const Ordinal& a, BigInt& outK);
    // accumulated fraction value for finite index k (k>=1 yields the sum).
    Rational valueFinite_(BigInt k) const;

public:
    Ns() { alpha_ = Ordinal::fromInt(1); }
    explicit Ns(const std::string& s) { string_to_it(s); }
    Ns(BigInt n, BigInt m) : n_(n), m_(m) { alpha_ = Ordinal::fromInt(1); }
    Ns(BigInt n, BigInt m, const std::string& s) : n_(n), m_(m) { string_to_it(s); }

    void setParams(BigInt n, BigInt m) { n_ = n; m_ = m; }
    std::pair<BigInt, BigInt> params() const { return {n_, m_}; }
    bool isDefaultParams() const { return n_ == 2 && m_ == 2; }

    std::string name() const override { return "ns"; }
    Family family() const override { return Family::Ordinal; }
    std::string subfamily() const override { return "ns"; }
    std::string style() const override { return "real_sequence"; }

    // 创造者 / creator: 送到本到 (UTF-8) & nnc (see spec/notations/ns.*.md).
    // 版本 / version: 1 (base version; 计划模程待最终确认格式).
    std::string creator() const override { return "送到本到, nnc"; }
    std::string version() const override { return "1"; }

    Capabilities capabilities() const override;

    void string_to_it(const std::string& s) override;
    std::string to_string() const override;

    // The notation expression "α th NS" (LaTeX-ish). The accumulated
    // fraction value is exposed separately (see value() / to_fraction_string()).
    // Delegates to core::Ordinal::expand (§11 pluggable FS); a finite /
    // successor index has no fundamental sequence, so expand throws
    // UnsupportedOperation (the "can't auto-compute" case the user noted).
    Ns& expand(BigInt k) override;

    int compare(const Notation& other) const override;

    // --- accumulated weight-fraction views (only meaningful for FINITE α) ---
    // Exact rational value x = α th NS for finite α; throws std::domain_error
    // for limit indices (not auto-computed).
    Rational value() const;
    // The accumulated weight-fraction form: int + w1/d1 + w2/d2 + ...
    // (the user's "累加权重分数"; e.g. "2 + 5/11 + 62/111"). For Ns
    // finite α the int part is 0 and each weight is 1 (terms 1/n^β).
    WeightedFractionSum accumulated_weight_fractions() const;
    // Rendered sum, e.g. "1/2 + 1/4" or "2 + 5/11 + 62/111"; "0" for
    // the first element.
    std::string to_fraction_string() const;
};

} // namespace ordinal
} // namespace googology
