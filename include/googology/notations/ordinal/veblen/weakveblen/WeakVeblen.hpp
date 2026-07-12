#pragma once
#include <string>
#include <vector>
#include "googology/core/Notation.hpp"
#include "googology/core/Capability.hpp"
#include "googology/core/Ordinal.hpp"

namespace googology {
namespace ordinal {

// Weak-Veblen-like notation (zahin). See spec/notations/weak_veblen_like.md.
//
// A = (a_1@b_1, ..., a_n@b_n) with a_i, b_i in On and b_i > b_{i+1}.
// Implements EXACTLY the expand rules in `study/notations/
// weak-Veblen-like notation.md` (the 6 cases). No added interpretation:
//   * expansion follows the article's 6-case dispatch verbatim; the ordinal
//     arithmetic inside (+1, ^, expand(b,n)) is supplied by the Ordinal
//     core (core/Ordinal.hpp), which itself follows `study/notations/Ordinal.md`.
//   * the article defines NO compare / normalize / successor clause for this
//     notation, so capabilities report only FromString / ToString / Expand.
//
// Internally holds an `Ordinal` (the (a@b) expression tree, or — after
// expansion — an ordinal expression the article's cases may produce, e.g.
// (#,a@0)+1 or the finite n). expand() rewrites that Ordinal and returns
// *this; to_string() renders it.
//
// NOTE (architecture flag): this derives straight from Notation, NOT from
// OrdinalNotation. The earlier project directive said "all ordinal notations
// inherit OrdinalNotation (standard-form detection)". But weak-Veblen is NOT
// a natural-number SEQUENCE notation — its entries are ordinal PAIRS (a@b) —
// and the article defines NO standard form for it. Following "照文章写" (do not
// add what the article omits), no normalize()/isSuccessor() is added. The
// user should decide whether a standard-form concept belongs here.
class WeakVeblen : public Notation {
    Ordinal ord_;   // the (a@b) expression tree

    // parse "a1@b1, a2@b2, ..." (already stripped of outer parens)
    void parseBody_(const std::string& body);

public:
    WeakVeblen() = default;
    explicit WeakVeblen(const std::string& s) { string_to_it(s); }
    explicit WeakVeblen(const std::vector<std::pair<Ordinal, Ordinal>>& c)
        : ord_(Ordinal::wv(c)) {}

    std::string name() const override { return "weak_veblen"; }
    Family family() const override { return Family::Ordinal; }
    std::string subfamily() const override { return "veblen"; }
    std::string style() const override { return "weak-like"; }

    Capabilities capabilities() const override;

    void string_to_it(const std::string& s) override;
    std::string to_string() const override;

    // Rewrites the internal Ordinal via the article's 6-case expand and
    // returns *this (a WeakVeblen object). expand_to()/A[n]-style length
    // expansion is NOT in the article -> left to the base class (throws
    // UnsupportedOperation).
    WeakVeblen& expand(BigInt n) override;

    // A[n] — the n-th term of the fundamental sequence of the ordinal A
    // denotes. Equivalent to expand(A, n). Does NOT mutate *this (returns
    // a copy), so repeated indexing A[1], A[2], ... is safe.
    WeakVeblen operator[](BigInt n) const;

    friend std::istream& operator>>(std::istream& is, WeakVeblen& p);
};

} // namespace ordinal
} // namespace googology
