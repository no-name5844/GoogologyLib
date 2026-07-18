#pragma once
#include <memory>
#include <string>
#include <vector>
#include "googology/core/Notation.hpp"
#include "googology/core/Capability.hpp"
#include "googology/core/Ordinal.hpp"
#include <stdexcept>

namespace googology {

// Ordinal notation base. EVERY ordinal notation (regardless of sub-family:
// sequence, CNF, Veblen, Buchholz, Taranovsky, ...) derives from this, which
// in turn derives from the generic Notation. This is where ordinal-specific
// behavior lives — in particular the standard form (标准型):
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
    // representation; its fundamental sequence is exactly the
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
    // Runs the reusable §12 decision engine WITHIN THIS notation system,
    // using only this notation's own compare() / expand() (its fundamental
    // sequence) / roots(). Member of the ordinal-notation class by design —
    // the judgment is an ordinal-notation concept.
    //
    // NON-VIRTUAL and UNIVERSAL: standard-form DETECTION is one single
    // implementation shared by EVERY ordinal notation. For any notation that
    // has a fundamental-sequence definition (which is all of them that matter
    // — Prss / ε_pSS / ε_ωSS / WeakVeblen / BMS / …), the §12 reachability
    // definition applies identically; there is NO per-notation distinction.
    // The engine needs only a fundamental sequence (expand) + a limit
    // expression (roots) + an order for pruning (compare). It deliberately
    // does NOT require a "true" ordinal order: a notation's own compare()
    // agrees with ordinal order on its STANDARD forms, which is all the
    // pruning needs. (Contrast normalize(), the rewriting ACTION, which is
    // virtual / per-notation.)
    bool is_standard() const;

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

// ===========================================================================
// §12 standard-form (标准表达式) decision engine.
// MERGED from the former standalone core/StandardForm.hpp into this header so
// the ordinal-notation class carries its own judgment logic in one place.
//
// Scoped to ORDINAL notations: it relies only on (a) a total ordinal order
// `cmp` and (b) a fundamental-sequence rule `fundSeq`, both of which exist
// ONLY for ordinal notations. Number notations (Knuth / Conway) and the real
// sequence Ns have no ordinal order / fundamental sequence, so §12 does not
// apply to them. The engine is templated on `Expr` (the expression type of
// ONE notation system) and needs only cmp / fundSeq / clone / roots.
// Two dual strategies from §12: is_standard_bfs (downward BFS, cmp-pruned)
// and is_standard_backtrack (upward). The library ships one concrete
// instantiation over the unified ordinal VALUE type `core::Ordinal`
// (class OrdinalSystem, used by the free is_standard_bfs(Ordinal) /
// is_standard_backtrack(Ordinal) convenience overloads). The ordinal
// notation class provides its OWN instantiation via an adapter
// (OrdinalNotationSystem) over its compare()/expand()/clone()/roots() and
// exposes the decision as the member function is_standard().
// ===========================================================================
template <class Expr>
struct StdSystem {
    virtual ~StdSystem() = default;
    virtual int                       cmp(const Expr& a, const Expr& b) const = 0;
    virtual Expr                      fundSeq(const Expr& a, long long n) const = 0;
    virtual Expr                      clone(const Expr& a) const = 0;
    virtual std::vector<Expr>        roots() const = 0;
};

namespace detail {
// does `x` have a fundamental sequence? (a limit does; 0 / a successor /
// a closed CNF ordinal do not — they throw on expand). Fundamental-sequence
// indices are 0-based project-wide: the first
// (smallest) sequence element is fundSeq(x, 0).
template <class Expr>
inline bool has_fs(const StdSystem<Expr>& s, const Expr& x) {
    try { (void)s.fundSeq(s.clone(x), 0); return true; }
    catch (...) { return false; }
}
template <class Expr>
inline Expr fs(const StdSystem<Expr>& s, const Expr& x, long long n) {
    return s.fundSeq(s.clone(x), n);
}

// Find the finite candidate-index band [n0, n1] for a MONOTONIC fund_seq
// (design.md §12.3). Returns false when no bounded band exists.
// Fundamental-sequence indices are 0-based project-wide: fundSeq(X, 0) is
// the smallest (minimal) sequence element, and the sequence is monotonically
// non-decreasing in n. n0 = smallest n with fundSeq(X, n) >= target;
// n1 = largest n (<= hi) with fundSeq(X, n) <= target.
template <class Expr>
inline bool band(const StdSystem<Expr>& s, const Expr& X, const Expr& target,
                long long maxProbe, long long& n0, long long& n1) {
    // floor = fundSeq(X, 0), the smallest element.
    Expr f0 = fs(s, X, 0);
    int c0 = s.cmp(f0, target);
    if (c0 > 0) { n0 = 0; n1 = -1; return false; }   // even smallest > target → unreachable
    if (c0 == 0) { n0 = 0; n1 = 0; return true; }    // smallest == target → band is [0,0]
    // f0 < target: double-probe an upper bound hi with fundSeq(X, hi) >= target.
    long long hi = 1;
    for (;;) {
        Expr h = fs(s, X, hi);
        if (s.cmp(h, target) >= 0) break;
        if (hi >= maxProbe) { hi = maxProbe; break; }
        hi *= 2;
        if (hi > maxProbe) hi = maxProbe;
    }
    Expr hF = fs(s, X, hi);
    if (hi >= maxProbe && s.cmp(hF, target) < 0) { n0 = maxProbe + 1; n1 = 0; return false; }
    // n0 = smallest n in [0, hi] with fundSeq(X, n) >= target
    long long a = 0, b = hi;
    while (a < b) {
        long long m = a + (b - a) / 2;
        if (s.cmp(fs(s, X, m), target) >= 0) b = m; else a = m + 1;
    }
    n0 = a;
    // n1 = largest n in [n0, hi] with fundSeq(X, n) <= target
    a = n0; b = hi;
    while (a < b) {
        long long m = a + (b - a + 1) / 2;
        if (s.cmp(fs(s, X, m), target) <= 0) a = m; else b = m - 1;
    }
    n1 = a;
    return n0 <= n1;
}
} // namespace detail

// §12.1 — downward BFS with cmp-based pruning.
template <class Expr>
inline bool is_standard_bfs(const StdSystem<Expr>& sys, const Expr& target,
                            long long maxProbe = (1LL << 18)) {
    using namespace detail;
    std::vector<Expr> Q;
    for (auto& r : sys.roots()) Q.push_back(sys.clone(r));
    std::vector<Expr> visited;
    auto seen = [&](const Expr& x) -> bool {
        for (auto& v : visited) if (sys.cmp(v, x) == 0) return true;
        return false;
    };
    while (!Q.empty()) {
        Expr X = Q.back(); Q.pop_back();
        if (seen(X)) continue;
        visited.push_back(sys.clone(X));
        int c = sys.cmp(X, target);
        if (c == 0) return true;
        if (c < 0) continue;
        if (!has_fs(sys, X)) continue;
        long long n0 = 0, n1 = 0;
        if (!band(sys, X, target, maxProbe, n0, n1)) {
            // band failed: even fundSeq(X, 0) (the smallest element) exceeds
            // target, so NO fundamental-sequence element of X can equal target
            // (the sequence is monotonic). Nothing to push — skip.
            continue;
        }
        for (long long n = n0; n <= n1; ++n) {
            Expr Y = fs(sys, X, n);
            if (!seen(Y)) Q.push_back(std::move(Y));
        }
    }
    return false;
}

// §12.2 — dual decision procedure (single descent search).
template <class Expr>
inline bool is_standard_backtrack(const StdSystem<Expr>& sys, const Expr& target) {
    using namespace detail;
    std::vector<Expr> Q;
    for (auto& r : sys.roots()) Q.push_back(sys.clone(r));
    std::vector<Expr> visited;
    auto seen = [&](const Expr& x) -> bool {
        for (auto& v : visited) if (sys.cmp(v, x) == 0) return true;
        return false;
    };
    while (!Q.empty()) {
        Expr X = Q.back(); Q.pop_back();
        if (seen(X)) continue;
        visited.push_back(sys.clone(X));
        int c = sys.cmp(X, target);
        if (c == 0) return true;
        if (c < 0) continue;
        if (!has_fs(sys, X)) continue;
        long long n0 = 0, n1 = 0;
        if (!band(sys, X, target, (1LL << 18), n0, n1)) {
            // band failed: even the smallest FS element > target → unreachable.
            continue;
        }
        for (long long n = n0; n <= n1; ++n) {
            Expr Y = fs(sys, X, n);
            if (sys.cmp(Y, target) == 0) return true;
            if (!seen(Y)) Q.push_back(std::move(Y));
        }
    }
    return false;
}

// Concrete ordinal system: core::Ordinal as the §12 system.
// Roots = {0, ω, ω^ω}. Under the article's fundamental sequences
// (ω[n] = n, (ω^ω)[n] = ω^n) the reachable standard ordinals are
//   {0, 1, 2, ..., ω, ω^2, ω^3, ..., ω^ω}.
class OrdinalSystem : public StdSystem<Ordinal> {
public:
    int                 cmp(const Ordinal& a, const Ordinal& b) const override { return a.compare(b); }
    Ordinal            fundSeq(const Ordinal& a, long long n) const override { return a.expand(n); }
    Ordinal            clone(const Ordinal& a) const override { return a; }
    std::vector<Ordinal> roots() const override {
        return { Ordinal::zero(),
                 Ordinal::omega(),
                 Ordinal::pow(Ordinal::omega(), Ordinal::omega()) };
    }
};

inline bool is_standard_bfs(const Ordinal& target) {
    OrdinalSystem sys; return is_standard_bfs<Ordinal>(sys, target);
}
inline bool is_standard_backtrack(const Ordinal& target) {
    OrdinalSystem sys; return is_standard_backtrack<Ordinal>(sys, target);
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
