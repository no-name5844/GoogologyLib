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
BigInt Prss::rightmostLess_(BigInt an) const {
    for (BigInt i = static_cast<BigInt>(seq_.size()) - 1; i >= 0; --i)
        if (seq_[static_cast<size_t>(i)] < an) return i + 1; // 1-based
    return 0; // none found: tail = whole sequence
}

// expandLen(A, M): expandLen(A,0) = (a_1..a_n-1); for M>=1, decrement the last
// element, then append M elements taken cyclically from the tail
// (columns br+1..n). See spec/notations/prss.md (⚠ cyclic wrap for totality).
Prss& Prss::expandLen_(BigInt M) {
    if (seq_.empty()) return *this;
    if (seq_.back() == 1) { seq_.pop_back(); return *this; } // ends with 1
    if (M <= 0) {
        if (seq_.back() > 0) seq_.back() -= 1; // expandLen(A,0)
        return *this;
    }
    BigInt n = static_cast<BigInt>(seq_.size());
    BigInt an = seq_.back();
    BigInt br = rightmostLess_(an);          // 1-based
    BigInt L = n - br;                       // tail length, >= 1
    // capture the tail (columns br+1..n, 0-based indices br..n-1) before mutating
    std::vector<BigInt> tail(static_cast<size_t>(L));
    for (BigInt i = 0; i < L; ++i)
        tail[static_cast<size_t>(i)] = seq_[static_cast<size_t>(br + i)];
    // expandLen(A,0): decrement the last element
    seq_[static_cast<size_t>(n - 1)] = an - 1;
    // append M elements, cycling within the tail
    for (BigInt step = 1; step <= M; ++step) {
        BigInt t = (step - 1) % L;
        seq_.push_back(tail[static_cast<size_t>(t)]);
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
    if (seq_.back() == 1) { seq_.pop_back(); return *this; } // ends with 1
    BigInt n = static_cast<BigInt>(seq_.size());
    BigInt an = seq_.back();
    BigInt br = rightmostLess_(an);
    BigInt L = n - br;
    if (m == 0) {
        if (an > 0) seq_[static_cast<size_t>(n - 1)] = an - 1;
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

bool Prss::isSuccessor() const {
    return !seq_.empty() && seq_.back() == 1;
}

BigInt Prss::expandUntilLarger_(Prss& work, const std::vector<BigInt>& target,
                                 size_t startIdx) {
    BigInt origLen = static_cast<BigInt>(work.seq_.size());
    BigInt val = work.seq_.back();
    BigInt targetVal = target[startIdx];
    if (origLen == static_cast<BigInt>(target.size()) && val == targetVal) return 0;
    if (val <= targetVal) return -1; // smaller than target -> non-standard
    if (val > targetVal + 1) {
        work.seq_.back() = targetVal + 1; // clamp down, then expand one step
    }
    // expand until the prefix (from startIdx) exceeds target, or length matches
    for (size_t i = static_cast<size_t>(origLen); i < target.size(); ++i) {
        work.expand(1);
        size_t last = work.seq_.size() - 1;
        if (work.seq_[last] > target[last]) return static_cast<BigInt>(work.seq_.size()) - origLen;
        if (work.seq_[last] < target[last]) return -1; // non-standard
        // equal: continue extending
    }
    return static_cast<BigInt>(work.seq_.size()) - origLen;
}

// Standard form (标准型), adapted from YSequence::checkStandardAndNonMaximum
// (YSequence.cpp). Builds a canonical PrSS from the input's second element and
// expands it until it dominates the input. ⚠ canonical starter [0, seq_[1]]
// (vs YSequence's [1, seq[1]+1]) and exact PrSS semantics need verification.
// The loop is guarded against stalls (work stops growing) so it always
// terminates; on a stall or a non-standard input, *this is left unchanged.
void Prss::normalize() {
    size_t n = seq_.size();
    if (n == 0) return;
    if (n == 1) { seq_[0] = 0; return; }       // standard: a_1 = 0
    if (n == 2) {
        seq_[0] = 0;
        if (seq_[1] < 0) seq_[1] = 0;
        return;
    }
    Prss work;
    // ⚠ canonical starter for PrSS: [0, seq_[1]] (base element is 0). Verify.
    work.seq_ = {0, seq_[1]};

    size_t idx = 0;
    const BigInt kGuard = 100000;
    BigInt steps = 0;
    while (idx < n) {
        // extend work until it covers index idx, but bail out if it stalls
        size_t guard = 0;
        while (work.seq_.size() <= idx) {
            std::vector<BigInt> before = work.seq_;
            work.expand(1);
            if (work.seq_ == before) return;       // stalled: leave unchanged
            if (++guard > static_cast<size_t>(kGuard)) return;
        }
        if (work.seq_[idx] < seq_[idx]) return;   // non-standard: left unchanged
        if (work.seq_[idx] == seq_[idx]) { idx++; continue; }
        // work.seq_[idx] > seq_[idx]: expand work until it exceeds the prefix
        BigInt added = expandUntilLarger_(work, seq_, idx);
        if (added == -1) return;                  // non-standard
        idx += static_cast<size_t>(added);
        if (++steps > kGuard) return;
    }
    seq_ = work.seq_;
}

std::istream& operator>>(std::istream& is, Prss& p) {
    std::string s;
    std::getline(is, s);
    if (!s.empty()) p.string_to_it(s);
    return is;
}

} // namespace ordinal
} // namespace googology
