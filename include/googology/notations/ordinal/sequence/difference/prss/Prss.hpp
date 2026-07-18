#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "googology/core/OrdinalNotation.hpp"
#include "googology/core/Capability.hpp"

namespace googology {
namespace ordinal {

// PrSS (Primitive Sequence System) — 阶差型 (difference) natural-number
// sequence notation. See spec/notations/prss.md.
//
// A sequence A = (a_1,...,a_n) satisfies: a_i in N, a_{i+1}-a_i <= 1, a_1 = 0,
// and a plateau is followed by a non-increase. The library NEVER evaluates to a
// number/ordinal: expand()/expand_to()/normalize() rewrite the internal
// sequence and return *this (a Prss object). to_string() emits LaTeX.
//
// Standard form (标准型) is computed by normalize(); see the project's
// universal definition (any expression obtainable from a limit expression by
// finitely many expansions + taking a prefix is a legal expression). compare()
// uses lexicographic ordinal order over the sequence.
class Prss : public OrdinalNotation {
    // rightmost 1-based column index whose value is < an (the last element);
    // returns 0 if none.
    BigInt rightmostLess_(BigInt an) const;
    // expandLen(A, M): decrement last, then append M elements per the
    // article's recursive rule (each indexed from the running sequence). Mutates *this.
    Prss& expandLen_(BigInt M);

public:
    Prss() = default;
    explicit Prss(const std::string& s) { string_to_it(s); }

    std::string name() const override { return "prss"; }
    Family family() const override { return Family::Ordinal; }
    std::string subfamily() const override { return "sequence"; }
    std::string style() const override { return "difference"; }

    Capabilities capabilities() const override;

    void string_to_it(const std::string& s) override;
    std::string to_string() const override;

    // These return the notation's OWN type (a rewritten Prss), not a string.
    Prss& expand(BigInt n) override;
    Prss& expand_to(BigInt len) override;

    // A[n] — the n-th term of the fundamental sequence of the ordinal A
    // denotes. Equivalent to expand(A, n). Does NOT mutate *this.
    Prss operator[](BigInt n) const;

    // --- §12 limit-expression API. ---
    // The n-th term of Prss's limit expression  LIMIT = (0,1,2,3,...):
    //   limit(0) = (),  limit(1) = (0),  limit(2) = (0,1),  ...,
    //   limit(n) = (0,1,...,n-1).
    // LIMIT.expand(m) == limit(m);  is_standard() seeds the §12 BFS
    // with LIMIT (via roots()).
    static Prss limit(BigInt n);
    // The master limit expression LIMIT itself (marked is_master_limit_).
    static Prss master_limit();

    // Lexicographic comparison (ordinal order over the sequence). Throws
    // NotComparable for non-Prss arguments. The master limit compares as
    // the supremum (greater than every finite sequence).
    int compare(const Notation& other) const override;

    // --- §12 standard-form decision support (OrdinalNotation member
    // is_standard() runs the generic engine over *this's own
    // compare()/expand()/roots()). ---
    OrdinalNotation* clone() const override { return new Prss(*this); }
    // Roots = the notation's master limit expression LIMIT = (0,1,2,...).
    // LIMIT.expand(m) == limit(m); it is the supremum of all standard
    // Prss expressions and seeds the downward §12 BFS.
    std::vector<std::shared_ptr<OrdinalNotation>> roots() const override {
        std::vector<std::shared_ptr<OrdinalNotation>> r;
        r.push_back(std::shared_ptr<OrdinalNotation>(new Prss(master_limit())));
        return r;
    }

    friend std::istream& operator>>(std::istream& is, Prss& p);
};

} // namespace ordinal
} // namespace googology
