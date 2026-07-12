#include "googology/notations/ordinal/sequence/difference/epsilonomegass/EpsOmegaSS.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>

namespace googology {
namespace ordinal {

Capabilities EpsOmegaSS::capabilities() const {
    Capabilities c;
    c.set(Op::FromString);
    c.set(Op::ToString);
    c.set(Op::Normalize);
    c.set(Op::Compare);
    c.set(Op::Expand);
    c.set(Op::ExpandTo);
    c.set(Op::Successor);
    return c;
}

BigInt EpsOmegaSS::rightmostLess_(BigInt an) const {
    for (BigInt i = static_cast<BigInt>(seq_.size()) - 1; i >= 0; --i)
        if (seq_[static_cast<size_t>(i)] < an) return i + 1;
    return 0;
}

// expandLen(A, M). VERBATIM per the article (no parameter p, no case 4):
//   m=0 -> (a_1..a_n - 1)                              [decrement last]
//   append step s=1..M (m = s):
//     q == 1       : append (s+n-L)th running      [case 2, no add]
//     q > 1 (any)  : append (n+s-1)th running + (q-1)  [case 3, q unbounded]
// "(X th expandLen(A,k))" is the X-th element (1-based) of the RUNNING,
// growing sequence at stage k -- NOT the original A. No wrap is defined by the
// article; an out-of-range index throws a defensive out_of_range (guard).
EpsOmegaSS& EpsOmegaSS::expandLen_(BigInt M) {
    if (seq_.empty()) return *this;
    if (seq_.back() == 1) { seq_.pop_back(); return *this; } // ends with 1 => successor
    BigInt n = static_cast<BigInt>(seq_.size());
    BigInt an = seq_.back();
    BigInt br1 = rightmostLess_(an);
    if (br1 == 0)
        throw std::out_of_range("EpsOmegaSS::expandLen_: no element < a_n (article undefined)");
    if (M <= 0) {
        if (an > 0) seq_.back() -= 1;
        return *this;
    }
    BigInt L = n - br1;
    BigInt q = an - seq_[static_cast<size_t>(br1 - 1)]; // a_n - a_br (original)
    seq_[static_cast<size_t>(n - 1)] = an - 1;
    for (BigInt s = 1; s <= M; ++s) {
        BigInt pos1, add;
        if (q == 1) {                 // case 2
            pos1 = s + n - L;
            add = 0;
        } else {                       // case 3 (q-1 added in full, unbounded)
            pos1 = n + s - 1;
            add = q - 1;
        }
        if (pos1 < 1 || pos1 > static_cast<BigInt>(seq_.size()))
            throw std::out_of_range("EpsOmegaSS::expandLen_: article index ("
                + std::to_string(pos1) + " th) out of range");
        seq_.push_back(seq_[static_cast<size_t>(pos1 - 1)] + add);
    }
    return *this;
}

void EpsOmegaSS::string_to_it(const std::string& s) {
    std::string t = s;
    t.erase(std::remove_if(t.begin(), t.end(), ::isspace), t.end());
    while (!t.empty() && (t.front() == '(' || t.front() == '[' || t.front() == '{'))
        t.erase(t.begin());
    while (!t.empty() && (t.back() == ')' || t.back() == ']' || t.back() == '}'))
        t.pop_back();
    seq_.clear();
    if (t.empty()) return;
    std::stringstream ss(t);
    std::string item;
    while (std::getline(ss, item, ',')) {
        if (item.empty()) continue;
        seq_.push_back(std::stoll(item));
    }
}

std::string EpsOmegaSS::to_string() const {
    std::ostringstream os;
    os << "(";
    for (size_t i = 0; i < seq_.size(); ++i) {
        if (i) os << ", ";
        os << seq_[i];
    }
    os << ")";
    return os.str();
}

EpsOmegaSS& EpsOmegaSS::expand(BigInt m) {
    if (seq_.empty()) return *this;
    if (seq_.back() == 1) { seq_.pop_back(); return *this; } // ends with 1 => successor
    BigInt n = static_cast<BigInt>(seq_.size());
    BigInt an = seq_.back();
    BigInt br1 = rightmostLess_(an);
    if (br1 == 0)
        throw std::out_of_range("EpsOmegaSS::expand: no element < a_n (article undefined)");
    BigInt L = n - br1;
    if (m == 0) {
        if (an > 0) seq_[static_cast<size_t>(n - 1)] = an - 1;
        return *this;
    }
    BigInt M = m * L - 1;
    return expandLen_(M);
}

EpsOmegaSS& EpsOmegaSS::expand_to(BigInt len) {
    return expandLen_(len);
}

EpsOmegaSS EpsOmegaSS::operator[](BigInt n) const {
    EpsOmegaSS tmp = *this;   // A[n] must NOT mutate *this
    tmp.expand(n);
    return tmp;
}

// Lexicographic ordinal comparison, valid when both operands are in standard
// form (user-specified property, consistent with Prss). Cross-type arguments
// throw NotComparable.
int EpsOmegaSS::compare(const Notation& other) const {
    const EpsOmegaSS* o = dynamic_cast<const EpsOmegaSS*>(&other);
    if (!o) throw NotComparable(name());
    size_t n1 = seq_.size(), n2 = o->seq_.size();
    size_t m = std::min(n1, n2);
    for (size_t i = 0; i < m; ++i) {
        if (seq_[i] > o->seq_[i]) return 1;
        if (seq_[i] < o->seq_[i]) return -1;
    }
    if (n1 > n2) return 1;
    if (n1 < n2) return -1;
    return 0;
}

std::istream& operator>>(std::istream& is, EpsOmegaSS& p) {
    std::string s;
    std::getline(is, s);
    if (!s.empty()) p.string_to_it(s);
    return is;
}

} // namespace ordinal
} // namespace googology
