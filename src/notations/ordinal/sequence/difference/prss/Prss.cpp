#include "googology/notations/ordinal/sequence/difference/prss/Prss.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>

namespace googology {
namespace ordinal {

Capabilities Prss::capabilities() const {
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

// rightmost 1-based column index whose value is strictly less than `an`.
// Returns 0 if none (tail = whole sequence).
BigInt Prss::rightmostLess_(BigInt an) const {
    for (BigInt i = static_cast<BigInt>(seq_.size()) - 1; i >= 0; --i)
        if (seq_[static_cast<size_t>(i)] < an) return i + 1; // 1-based
    return 0; // none found: tail = whole sequence
}

// expandLen(A, M). VERBATIM per the PrSS article:
//   m=0 -> (a_1..a_n - 1)                       [decrement last]
//   m=k+1 -> expandLen(A,k) ⊕ (m+n-L th expandLen(A,k))
// PrSS's Δ<=1 + plateau rule force the gap a_n - a_br to be exactly 1, so
// there is a single recursive case. The indexed element is taken from the
// RUNNING (growing) sequence expandLen(A,k), NOT the original A.
Prss& Prss::expandLen_(BigInt M) {
    if (seq_.empty()) return *this;
    if (seq_.back() == 0) { seq_.pop_back(); return *this; } // ends with 0 => successor
    if (M <= 0) {
        if (seq_.back() > 0) seq_.back() -= 1; // expandLen(A,0) = (a_1..a_n-1)
        return *this;
    }
    BigInt n = static_cast<BigInt>(seq_.size());
    BigInt an = seq_.back();
    BigInt br = rightmostLess_(an);          // 1-based
    if (br == 0) throw std::out_of_range("Prss::expandLen_: no element < a_n (article undefined)");
    BigInt L = n - br;                       // tail length, >= 1
    // expandLen(A,0): decrement the last element
    seq_[static_cast<size_t>(n - 1)] = an - 1;
    // append M elements; the s-th (m = s) is the (s+n-L)th element of the
    // running sequence (1-based).
    for (BigInt s = 1; s <= M; ++s) {
        BigInt pos1 = s + n - L;
        if (pos1 < 1 || pos1 > static_cast<BigInt>(seq_.size()))
            throw std::out_of_range("Prss::expandLen_: article index ("
                + std::to_string(pos1) + " th) out of range");
        seq_.push_back(seq_[static_cast<size_t>(pos1 - 1)]);
    }
    return *this;
}

void Prss::string_to_it(const std::string& s) {
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

std::string Prss::to_string() const {
    std::ostringstream os;
    os << "(";
    for (size_t i = 0; i < seq_.size(); ++i) {
        if (i) os << ", ";
        os << seq_[i];
    }
    os << ")";
    return os.str();
}

Prss& Prss::expand(BigInt m) {
    if (seq_.empty()) return *this;
    if (seq_.back() == 0) { seq_.pop_back(); return *this; } // ends with 0 => successor
    BigInt n = static_cast<BigInt>(seq_.size());
    BigInt an = seq_.back();
    BigInt br = rightmostLess_(an);
    if (br == 0) throw std::out_of_range("Prss::expand: no element < a_n (article undefined)");
    BigInt L = n - br;
    if (m == 0) {
        if (an > 0) seq_[static_cast<size_t>(n - 1)] = an - 1; // expand(A,0) = expandLen(A,0)
        return *this;
    }
    // expand(A, m>0) = expandLen(A, m*L - 1)
    BigInt M = m * L - 1;
    return expandLen_(M);
}

// expand_to(len) maps to expandLen(A, len): decrement last, append `len` tail
// elements. (Length-parameterized expansion; see spec.)
Prss& Prss::expand_to(BigInt len) {
    return expandLen_(len);
}

Prss Prss::operator[](BigInt n) const {
    Prss tmp = *this;   // A[n] must NOT mutate *this
    tmp.expand(n);
    return tmp;
}

int Prss::compare(const Notation& other) const {
    const Prss* o = dynamic_cast<const Prss*>(&other);
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

std::istream& operator>>(std::istream& is, Prss& p) {
    std::string s;
    std::getline(is, s);
    if (!s.empty()) p.string_to_it(s);
    return is;
}

} // namespace ordinal
} // namespace googology
