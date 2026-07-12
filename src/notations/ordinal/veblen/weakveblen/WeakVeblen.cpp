#include "googology/notations/ordinal/veblen/weakveblen/WeakVeblen.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>

namespace googology {
namespace ordinal {

Capabilities WeakVeblen::capabilities() const {
    Capabilities c;
    c.set(Op::FromString);
    c.set(Op::ToString);
    c.set(Op::Compare);   // article §1.3 now supplies is_equal/is_greater/compare
    c.set(Op::Expand);
    // The article defines no normalize / successor clause for this notation,
    // so those stay omitted (base class throws UnsupportedOperation).
    return c;
}

// ===========================================================================
// compare — article §1.3 (verbatim semantics)
//
// The article defines, for
//   A = (a1@i1, a2@i2, ..., an@in),  B = (b1@j1, b2@j2, ..., bm@jm),
// the predicates is_equal / is_greater and compare = {1,0,-1} accordingly.
// The comparison is LEXICOGRAPHIC with PRIMARY key the SECOND coordinate
// (i_k / j_k, the "@b" part) and SECONDARY key the FIRST coordinate
// (a_k / b_k, the "@a" part); when the compared components are equal the
// longer expression wins (prefix rule). We implement it directly as a 3-way
// compare over the component lists.
//
// NOTE (flag C5 — faithful reading of intent): the article writes the
// is_greater catch-alls as
//     false  if i1 <= j1
//     false  if a1 <= b1, i1 = j1
// Taken literally as top-to-bottom guards, the recursion case
// (is_greater(tail) for n,m>1) becomes UNREACHABLE, because i1=j1
// satisfies "i1 <= j1" and would return false before recursing. The intent
// is clearly "strictly less -> false, equal -> keep comparing", so we use
// '<' at those two points. This is the only deviation, and it merely makes
// the article's own recursion reachable — no new behaviour is added.
// ===========================================================================

// Compare two WV expressions (both must be kind WV). Defined as a
// static member so `friend class WeakVeblen` grants access to the
// private Ordinal::comps().
int WeakVeblen::cmpWV_(const Ordinal& A, const Ordinal& B) {
    if (A.kind() != Ordinal::Kind::WV || B.kind() != Ordinal::Kind::WV)
        throw std::domain_error(
            "WeakVeblen::compare: both operands must be WV expressions");
    return cmpLists_(A.comps(), B.comps());
}

// Lexicographic 3-way compare of two component lists.
//   primary   : second coordinate (i_k / j_k  ==  comps_[k].second)
//   secondary : first  coordinate (a_k / b_k  ==  comps_[k].first)
//   tie-break : longer list wins when the shorter is a prefix.
int WeakVeblen::cmpLists_(const std::vector<std::pair<Ordinal, Ordinal>>& Ca,
                          const std::vector<std::pair<Ordinal, Ordinal>>& Cb) {
    size_t n = Ca.size(), m = Cb.size();
    size_t i = 0;
    while (i < n && i < m) {
        int c2 = cmpOrd_(Ca[i].second, Cb[i].second); // primary: i (the @b)
        if (c2 != 0) return c2 > 0 ? 1 : -1;
        int c1 = cmpOrd_(Ca[i].first, Cb[i].first);   // secondary: a (the @a)
        if (c1 != 0) return c1 > 0 ? 1 : -1;
        ++i;
    }
    if (n > m) return 1;
    if (n < m) return -1;
    return 0;
}

// Compare two coordinate ordinals.
//   both WV       -> recurse the WV compare (the article allows i_k / a_k
//                    themselves to be weak-Veblen expressions, e.g. (0), (1@(0))).
//   both closed   -> ordinal (Cantor normal form) compare.
//   mixed         -> not defined by the article (non-standard form).
int WeakVeblen::cmpOrd_(const Ordinal& X, const Ordinal& Y) {
    bool xw = (X.kind() == Ordinal::Kind::WV);
    bool yw = (Y.kind() == Ordinal::Kind::WV);
    if (xw && yw) return cmpWV_(X, Y);
    if (xw || yw)
        throw std::domain_error(
            "WeakVeblen::compare: mixed WV / ordinal coordinate "
            "(non-standard form)");
    return X.compare(Y);   // both closed ordinals
}

int WeakVeblen::compare(const Notation& other) const {
    const WeakVeblen* o = dynamic_cast<const WeakVeblen*>(&other);
    if (!o) throw NotComparable(name());
    // The article's compare is defined on WV standard-form expressions.
    if (ord_.kind() == Ordinal::Kind::WV && o->ord_.kind() == Ordinal::Kind::WV)
        return cmpWV_(ord_, o->ord_);
    // If neither is a WV node (e.g. both reduced to closed ordinals),
    // fall back to ordinal comparison.
    if (ord_.kind() != Ordinal::Kind::WV && o->ord_.kind() != Ordinal::Kind::WV)
        return ord_.compare(o->ord_);
    // Mixed WV / ordinal -> not defined by the article.
    throw NotComparable(name());
}

// Split `body` (no outer parens) on TOP-LEVEL commas into "a@b" pieces,
// then split each piece on its TOP-LEVEL '@' into the a / b ordinals.
void WeakVeblen::parseBody_(const std::string& body) {
    std::vector<std::pair<Ordinal, Ordinal>> out;
    size_t i = 0, n = body.size();
    while (i < n) {
        while (i < n && body[i] == ' ') ++i;     // skip spaces
        if (i >= n) break;
        int depth = 0;
        std::string comp;
        while (i < n) {
            char c = body[i];
            if (c == '(') ++depth;
            else if (c == ')') { if (depth > 0) --depth; }
            else if (c == ',' && depth == 0) break;
            comp += c;
            ++i;
        }
        if (i < n && body[i] == ',') ++i;       // consume comma
        // split comp on top-level '@'
        int d2 = 0;
        size_t at = std::string::npos;
        for (size_t j = 0; j < comp.size(); ++j) {
            char c = comp[j];
            if (c == '(') ++d2;
            else if (c == ')') { if (d2 > 0) --d2; }
            else if (c == '@' && d2 == 0) { at = j; break; }
        }
        std::string as, bs;
        if (at == std::string::npos) { as = comp; bs = "0"; }   // missing @b -> b=0
        else { as = comp.substr(0, at); bs = comp.substr(at + 1); }
        Ordinal a = Ordinal::parse(as);
        Ordinal b = Ordinal::parse(bs);
        out.push_back({a, b});
    }
    ord_ = Ordinal::wv(out);
}

void WeakVeblen::string_to_it(const std::string& s) {
    std::string t = s;
    t.erase(std::remove_if(t.begin(), t.end(), ::isspace), t.end());
    while (!t.empty() && (t.front() == '(' || t.front() == '[' || t.front() == '{'))
        t.erase(t.begin());
    while (!t.empty() && (t.back() == ')' || t.back() == ']' || t.back() == '}'))
        t.pop_back();
    if (t.empty()) {
        // the zero notation: a single (0@0) -> article's case 1 A=(0)
        ord_ = Ordinal::wv({{Ordinal::zero(), Ordinal::zero()}});
        return;
    }
    parseBody_(t);
}

std::string WeakVeblen::to_string() const {
    return ord_.to_string();
}

WeakVeblen& WeakVeblen::expand(BigInt n) {
    ord_ = ord_.expand(static_cast<long long>(n));
    return *this;
}

WeakVeblen WeakVeblen::operator[](BigInt n) const {
    WeakVeblen tmp = *this;   // A[n] must NOT mutate *this
    tmp.expand(n);
    return tmp;
}

std::istream& operator>>(std::istream& is, WeakVeblen& p) {
    std::string s;
    std::getline(is, s);
    if (!s.empty()) p.string_to_it(s);
    return is;
}

} // namespace ordinal
} // namespace googology
