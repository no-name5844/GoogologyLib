#include "googology/notations/ordinal/sequence/difference/epspss/EpspSS.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>

namespace googology {
namespace ordinal {

Capabilities EpspSS::capabilities() const {
    Capabilities c;
    c.set(Op::FromString);
    c.set(Op::ToString);
    c.set(Op::Expand);
    c.set(Op::ExpandTo);
    return c;
}

// rightmost 1-based column index whose value is strictly less than `an`.
// Returns 0 only when an == 0 (i.e. the whole sequence is 0).
BigInt EpspSS::rightmostLess_(BigInt an) const {
    for (BigInt i = static_cast<BigInt>(seq_.size()) - 1; i >= 0; --i)
        if (seq_[static_cast<size_t>(i)] < an) return i + 1; // 1-based
    return 0;
}

// expandLen(A, M). Implements the article's formula VERBATIM:
//   m=0 -> (a_1..a_n - 1)                              [decrement last]
//   append step s=1..M (m = s in the article's expandLen(A,m)):
//     q = a_n - a_br
//     q == 1            : append (s+n-L)th A            [case 2, no add]
//     1 < q <= p        : append (n+s-1)th A + q       [case 3]
//     q > p             : append (n+s-1)th A + p       [case 4, capped]
// "(X th A)" is the X-th element of the ORIGINAL A (1-based), accessed
// literally. The article does NOT define a wrap; an out-of-range index is
// left as written and a defensive out_of_range is thrown (this is a guard,
// not a reinterpretation of the formula).
EpspSS& EpspSS::expandLen_(BigInt M) {
    if (seq_.empty()) return *this;
    if (seq_.back() == 1) { seq_.pop_back(); return *this; } // ends with 1
    if (seq_.back() == 0) return *this;                  // degenerate all-zero
    if (M <= 0) {
        if (seq_.back() > 0) seq_.back() -= 1;          // expandLen(A,0)
        return *this;
    }
    BigInt n = static_cast<BigInt>(seq_.size());
    BigInt an = seq_.back();
    BigInt br1 = rightmostLess_(an);   // >=1 since a_1 = 0 < an
    BigInt L = n - br1;
    std::vector<BigInt> A = seq_;                        // ORIGINAL A (before mutation)
    BigInt q = an - A[static_cast<size_t>(br1 - 1)];  // a_br = A[br1-1]
    seq_[static_cast<size_t>(n - 1)] = an - 1;         // expandLen(A,0): decrement last
    for (BigInt s = 1; s <= M; ++s) {
        BigInt pos1, add;
        if (q == 1) {                 // case 2
            pos1 = s + n - L;         // (m+n-L)th A, m = s
            add = 0;
        } else if (q <= p_) {         // case 3
            pos1 = n + s - 1;         // (n+m-1)th A, m = s
            add = q;
        } else {                       // case 4 (capped at p)
            pos1 = n + s - 1;         // (n+m-1)th A, m = s
            add = p_;
        }
        if (pos1 < 1 || pos1 > n)
            throw std::out_of_range("EpspSS::expandLen_: article index ("
                + std::to_string(pos1) + " th A) is out of range for the current sequence");
        seq_.push_back(A[static_cast<size_t>(pos1 - 1)] + add);
    }
    return *this;
}

void EpspSS::string_to_it(const std::string& s) {
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

std::string EpspSS::to_string() const {
    std::ostringstream os;
    os << "(";
    for (size_t i = 0; i < seq_.size(); ++i) {
        if (i) os << ", ";
        os << seq_[i];
    }
    os << ")";
    return os.str();
}

EpspSS& EpspSS::expand(BigInt m) {
    if (seq_.empty()) return *this;
    if (seq_.back() == 1) { seq_.pop_back(); return *this; } // ends with 1
    BigInt n = static_cast<BigInt>(seq_.size());
    BigInt an = seq_.back();
    BigInt br1 = rightmostLess_(an);
    BigInt L = n - br1;
    if (m == 0) {
        if (an > 0) seq_[static_cast<size_t>(n - 1)] = an - 1; // expand(A,0)
        return *this;
    }
    BigInt M = m * L - 1;             // expand(A, m>0) = expandLen(A, m*L-1)
    return expandLen_(M);
}

// expand_to(len) maps to expandLen(A, len): decrement last, append `len`
// elements per the article's case rules.
EpspSS& EpspSS::expand_to(BigInt len) {
    return expandLen_(len);
}

std::istream& operator>>(std::istream& is, EpspSS& p) {
    std::string s;
    std::getline(is, s);
    if (!s.empty()) p.string_to_it(s);
    return is;
}

} // namespace ordinal
} // namespace googology
