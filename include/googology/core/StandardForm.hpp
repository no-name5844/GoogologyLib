#pragma once
// Standard-form (标准表达式) membership decision — doc/design.md §12,
// implemented as real, reusable code, SCOPED TO ORDINAL NOTATIONS.
//
// The standard-form judgment is fundamentally an ORDINAL-notation concept: it
// relies only on (a) a total ordinal order `cmp` and (b) a fundamental-
// sequence rule `fundSeq`, both of which exist ONLY for ordinal notations.
// It is NOT a generic property of arbitrary expressions — Knuth up-arrows,
// Conway chains, Ns (a real sequence indexed by an ordinal), etc. have no
// ordinal order / fundamental sequence, so §12 does not apply to them.
//
// This module is the GENERIC engine. It is templated on `Expr` (the
// expression type of ONE notation system) and needs ONLY:
//   * cmp(a,b)       -> -1/0/1   (total ordinal order on standard exprs)
//   * fundSeq(a,n)   -> the n-th fundamental-sequence term of a (throws if a
//                          has no fundamental sequence)
//   * clone(a)       -> a fresh copy
//   * roots()        -> a finite set of "top-level" limit expressions
// No external semantics are used; the result is system-relative.
//
// Two mutually-dual strategies from §12:
//   * is_standard_bfs()       — downward BFS from roots, pruned by cmp
//   * is_standard_backtrack() — upward backtracking from the target
//
// The library ships ONE concrete instantiation of this engine over the unified
// ordinal VALUE type `core::Ordinal` (class OrdinalSystem, used by the
// free `is_standard_bfs(Ordinal)` / `is_standard_backtrack(Ordinal)`
// convenience overloads). The ORDINAL NOTATION CLASS `OrdinalNotation`
// (core/OrdinalNotation.hpp) provides its OWN instantiation via an adapter
// over its own `compare()` / `expand()` / `clone()` / `roots()` and
// exposes the decision as a MEMBER FUNCTION `is_standard()` — so the
// judgment lives where the user placed it: inside the ordinal-notation
// class, operating within that single notation system (using only the
// system's own cmp + fundSeq), exactly as §12 states.
#include <stdexcept>
#include <vector>
#include "googology/core/Ordinal.hpp"

namespace googology {

// Minimal ORDINAL system interface the decision needs (design.md §12).
// Concrete ordinal-notation systems subclass this and supply only compare /
// fundamental sequence / clone / roots. `Expr` is the expression type of
// the single notation system under judgment (e.g. core::Ordinal, or
// std::shared_ptr<OrdinalNotation> for the ordinal-notation classes).
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
// a closed CNF ordinal do not — they throw on expand).
template <class Expr>
inline bool has_fs(const StdSystem<Expr>& s, const Expr& x) {
    try { (void)s.fundSeq(s.clone(x), 1); return true; }
    catch (...) { return false; }
}
template <class Expr>
inline Expr fs(const StdSystem<Expr>& s, const Expr& x, long long n) {
    return s.fundSeq(s.clone(x), n);
}

// Find the finite candidate-index band [n0, n1] for a MONOTONIC fund_seq
// (design.md §12.3). Returns false when no bounded band exists (a non-
// monotonic system must supply candidates by syntax analysis instead — see
// §12.3 tail; the decision algorithm is otherwise not directly implementable
// on that system).
template <class Expr>
inline bool band(const StdSystem<Expr>& s, const Expr& X, const Expr& target,
                long long maxProbe, long long& n0, long long& n1) {
    Expr f1 = fs(s, X, 1);
    int c1 = s.cmp(f1, target);
    if (c1 > 0) { n0 = 1; n1 = 0; return false; }   // target below smallest term
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
    // n0 = first n in [1, hi] with cmp >= 0
    long long a = 1, b = hi;
    while (a < b) {
        long long m = a + (b - a) / 2;
        if (s.cmp(fs(s, X, m), target) >= 0) b = m; else a = m + 1;
    }
    n0 = a;
    // n1 = last n in [n0, hi] with cmp <= 0
    a = n0; b = hi;
    while (a < b) {
        long long m = a + (b - a + 1) / 2;
        if (s.cmp(fs(s, X, m), target) <= 0) a = m; else b = m - 1;
    }
    n1 = a;
    return n0 <= n1;
}
} // namespace detail

// ---------------------------------------------------------------------------
// §12.1 — downward BFS with cmp-based pruning.
// ---------------------------------------------------------------------------
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
        if (c == 0) return true;     // EQUAL — found
        if (c < 0) continue;         // LESS — only shrinks further, prune

        // c > 0 (GREATER): may still reach target by expansion, if X is a limit.
        if (!has_fs(sys, X)) continue;   // not a limit -> discard
        long long n0 = 0, n1 = 0;
        if (!band(sys, X, target, maxProbe, n0, n1)) {
            // band() false: target is either below X[1] or above maxProbe.
            // If BELOW X[1], X[1] is still a strict (well-founded)
            // descendant whose own expansion subtree may reach target -- push
            // it so that subtree is explored. (This is what makes the 0-th /
            // bottom term of a limit expression, e.g. Prss limit(0)=(),
            // reachable, even though the engine indexes FS from 1.)
            if (sys.cmp(fs(sys, X, 1), target) > 0) {
                Expr Y = fs(sys, X, 1);
                if (!seen(Y)) Q.push_back(std::move(Y));
            }
            continue;
        }
        for (long long n = n0; n <= n1; ++n) {
            Expr Y = fs(sys, X, n);
            if (!seen(Y)) Q.push_back(std::move(Y));
        }
    }
    return false;   // Q drained, target never equalled -> non-standard
}

// ---------------------------------------------------------------------------
// §12.2 — the dual decision procedure, phrased as a single descent search.
// A target is standard iff it is reachable from some ROOT by finitely many
// expansions (taking fundamental-sequence terms). This is the same
// well-founded reachability as §12.1; the "upward" reading is: if
// target == fundSeq(X, n) for a reachable limit X, then target is
// standard because X is. The band() "below X[1]" case descends into
// X[1] so the 0-th / bottom term of a limit expression is reached.
// ---------------------------------------------------------------------------
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
        if (c == 0) return true;     // X itself equals target -> reachable
        if (c < 0) continue;         // X < target, expansion only shrinks
        if (!has_fs(sys, X)) continue;
        long long n0 = 0, n1 = 0;
        if (!band(sys, X, target, (1LL << 18), n0, n1)) {
            // below X[1]: descend into X[1] (smallest FS term) so the
            // well-founded subtree (which may contain target) is explored.
            if (sys.cmp(fs(sys, X, 1), target) > 0) {
                Expr Y = fs(sys, X, 1);
                if (sys.cmp(Y, target) == 0) return true;   // target == X[1]
                if (!seen(Y)) Q.push_back(std::move(Y));
            }
            continue;
        }
        for (long long n = n0; n <= n1; ++n) {
            Expr Y = fs(sys, X, n);
            if (sys.cmp(Y, target) == 0) return true;       // target == X[n]
            if (!seen(Y)) Q.push_back(std::move(Y));
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Concrete ordinal system: core::Ordinal as the §12 system.
// Roots = {0, ω, ω^ω}. 0 is the base standard expression (ω[n]=n yields
// 0 at n=0, but we list it explicitly as a root so the decision has a
// finite standard base to start from). Under the article's fundamental
// sequences (ω[n] = n, (ω^ω)[n] = ω^n) the reachable standard ordinals
// are
//   {0, 1, 2, ..., ω, ω^2, ω^3, ..., ω^ω}.
// An ordinal such as ω+1 / ω·2 / ω^2+ω is NOT generated by this system's
// roots+FS, so it is reported non-standard *within this system* — exactly the
// system-relativity §12 states. Widen roots / supply a stronger FS (e.g. a
// full Cantor-normal-form FS, or add ω·2 / ε0 as roots) to cover more
// ordinals.
// ---------------------------------------------------------------------------
class OrdinalSystem : public StdSystem<Ordinal> {
public:
    int                cmp(const Ordinal& a, const Ordinal& b) const override { return a.compare(b); }
    Ordinal           fundSeq(const Ordinal& a, long long n) const override { return a.expand(n); }
    Ordinal           clone(const Ordinal& a) const override { return a; }   // value type
    std::vector<Ordinal> roots() const override {
        return { Ordinal::zero(),
                 Ordinal::omega(),
                 Ordinal::pow(Ordinal::omega(), Ordinal::omega()) };
    }
};

// Convenience: judge an Ordinal with the default OrdinalSystem.
inline bool is_standard_bfs(const Ordinal& target) {
    OrdinalSystem sys; return is_standard_bfs<Ordinal>(sys, target);
}
inline bool is_standard_backtrack(const Ordinal& target) {
    OrdinalSystem sys; return is_standard_backtrack<Ordinal>(sys, target);
}

} // namespace googology
