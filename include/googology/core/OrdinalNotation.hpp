#pragma once
#include <memory>
#include <string>
#include <vector>
#include "googology/core/Notation.hpp"
#include "googology/core/Capability.hpp"
#include "googology/core/StandardForm.hpp"

namespace googology {

// Ordinal notation base. EVERY ordinal notation (regardless of sub-family:
// sequence, CNF, Veblen, Buchholz, Taranovsky, ...) derives from this, which
// in turn derives from the generic Notation. This is where ordinal-specific
// behavior lives — in particular the standard form (标准型) the project
// discussed:
//
//   * The *definition* of standard form is UNIVERSAL — it is the single
//     UMBRELLA concept, shared by ALL ordinal notations. Formally:
//       一个表达式是【标准表达式】 ⇔ 它可以通过某个【极限表达式】，
//       经【有限次展开（expand）】并【取基本列前若干项】的方式得到。
//     (an expression is standard iff obtainable from a limit expression by
//      finitely many expansions + taking a prefix of the fundamental sequence)
//   * The *action* of normalizing (规范化 / standard-form-ization) is
//     PER-NOTATION: each notation rewrites its own expressions to the universal
//     standard form. It is NOT an umbrella function. The shared body below is
//     only a convenience default for the ordinal SEQUENCE notations (they happen
//     to share the same algorithm, differing only by `baseVal_`: PrSS = 0,
//     ε_pSS / ε_ωSS = 1); conceptually normalize() belongs to each notation.
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
    // Marks THIS object as the notation's MASTER LIMIT EXPRESSION
    // (design.md §12 "LIMIT"). A master limit has no finite sequence
    // representation; its fundamental sequence is exactly the user-defined
    // limit(n):  LIMIT.expand(m) == limit(m).  compare() treats it as
    // the supremum (greater than every finite sequence). roots() seeds
    // the §12 BFS with a fresh master limit, so the engine operates
    // within the single notation system, using only this notation's own
    // cmp + fund_seq. Copied by the default copy constructor / clone().
    bool is_master_limit_ = false;

public:
    // Standard form (标准型) — the DEFINITION above is the universal umbrella.
    // normalize() (规范化) is conceptually PER-NOTATION: each notation owns
    // its own rewriting. Declared virtual so notations may override it; the
    // shared body here is a default for the ordinal-sequence family (only
    // baseVal_ differs). On a stall or a non-standard input, *this is left
    // unchanged.
    virtual void normalize();

    // Successor test — shared, NON-virtual. A sequence is a successor iff
    // it ends with baseVal_ (the article's successor clause).
    bool isSuccessor() const;

    // Polymorphic copy, used by the §12 standard-form decision engine
    // to clone expressions without slicing. Each concrete ordinal
    // notation overrides it, e.g. `return new Prss(*this);`.
    virtual OrdinalNotation* clone() const = 0;

    // Base standard / top-level limit expressions of THIS notation system
    // (design.md §12 "Roots"). The decision BFS / backtracking
    // starts here. Each concrete ordinal notation overrides it (the
    // natural-number sequence notations use their limit seed; the exact
    // limit expression follows the article for that notation).
    virtual std::vector<std::shared_ptr<OrdinalNotation>> roots() const = 0;

    // Detect whether *this is in standard form (标准型), per design.md §12.
    // Runs the reusable §12 decision engine WITHIN THIS notation
    // system, using only this notation's own compare() / expand() (its
    // fundamental sequence) / roots(). Member of the ordinal-notation
    // class by design — the judgment is an ordinal-notation concept.
    // DECLARED VIRTUAL: the decision is PER-NOTATION. The ordinal
    // SEQUENCE notations (Prss / ε_pSS / ε_ωSS) use the generic
    // expansion-reachability engine above; a matrix notation like BMS
    // overrides it with its own direct syntactic check (§0.1 of its
    // article). Making it virtual keeps the "标准型判定属于序数记号
    // 这个类" principle uniform across all ordinal notations.
    virtual bool is_standard() const;

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
    if (is_master_limit_) return;            // master limit has no normal form
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

// ---------------------------------------------------------------------------
// Adapter: expose an OrdinalNotation instance as a §12 StdSystem over
// std::shared_ptr<OrdinalNotation> (polymorphic, no slicing). compare
// uses the notation's OWN virtual compare(); the fundamental sequence is
// the notation's OWN expand() applied to a clone; roots delegate to the
// notation's OWN roots(). This is exactly the "single notation system,
// own cmp + fund_seq" framing of §12 — so is_standard() (below) runs
// the generic engine within THIS notation, not via any external ordinal.
// ---------------------------------------------------------------------------
class OrdinalNotationSystem : public StdSystem<std::shared_ptr<OrdinalNotation>> {
    const OrdinalNotation* self_;
public:
    explicit OrdinalNotationSystem(const OrdinalNotation* s) : self_(s) {}
    int cmp(const std::shared_ptr<OrdinalNotation>& a,
            const std::shared_ptr<OrdinalNotation>& b) const override {
        return a->compare(*b);
    }
    std::shared_ptr<OrdinalNotation> fundSeq(const std::shared_ptr<OrdinalNotation>& a,
                                                long long n) const override {
        auto y = std::shared_ptr<OrdinalNotation>(a->clone());
        y->expand(n);
        return y;
    }
    std::shared_ptr<OrdinalNotation> clone(const std::shared_ptr<OrdinalNotation>& a) const override {
        return std::shared_ptr<OrdinalNotation>(a->clone());
    }
    std::vector<std::shared_ptr<OrdinalNotation>> roots() const override {
        return self_->roots();
    }
};

inline bool OrdinalNotation::is_standard() const {
    OrdinalNotationSystem sys(this);
    auto target = std::shared_ptr<OrdinalNotation>(this->clone());
    return is_standard_bfs(sys, target);
}

} // namespace googology
