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
    c.set(Op::Normalize);
    c.set(Op::Compare);
    c.set(Op::Expand);
    c.set(Op::ExpandTo);
    c.set(Op::Successor);
    return c;
}

// rightmost 1-based column index whose value is strictly less than `an`.
BigInt EpspSS::rightmostLess_(BigInt an) const {
    for (BigInt i = static_cast<BigInt>(seq_.size()) - 1; i >= 0; --i)
        if (seq_[static_cast<size_t>(i)] < an) return i + 1; // 1-based
    return 0; // none found: tail = whole sequence
}

// expandLen(A, M). Unified tail-wrap: appended index is (n+m-1) (1-based,
// wraps cyclically in the tail [br+1..n]); the added amount depends on the
// gap q = a_n - a_{br}: 0 for q==1 (case 2), q for 1<q<=p (case 3),
// p for q>p (case 4, capped). See spec/notations/epsilon_p_ss.md (⚠ closure).
EpspSS& EpspSS::expandLen_(BigInt M) {
    if (seq_.empty()) return *this;
    if (seq_.back() == 1) { seq_.pop_back(); return *this; } // ends with 1
    if (M <= 0) {
        if (seq_.back() > 0) seq_.back() -= 1; // expandLen(A,0)
        return *this;
    }
    BigInt n = static_cast<BigInt>(seq_.size());
    BigInt an = seq_.back();
    if (an == 0) return *this;                  // ⚠ degenerate all-zero guard
    BigInt br1 = rightmostLess_(an);             // 1-based, >=1 since a_1=0<an
    BigInt L = n - br1;                          // tail length, >= 1
    // capture the original tail (0-based [br1, n-1]) BEFORE mutating
    std::vector<BigInt> tail(static_cast<size_t>(L));
    for (BigInt i = 0; i < L; ++i)
        tail[static_cast<size_t>(i)] = seq_[static_cast<size_t>(br1 + i)];
    // gap q = a_n - a_{br} (br1 is rightmost < an, so q >= 1)
    BigInt q = an - seq_[static_cast<size_t>(br1 - 1)];
    // choose mode (epsilon_p)
    int add; // amount to add to the appended tail element
    if (q == 1)       add = 0;   // case 2: (n+m-1 th A)
    else if (q <= p_)  add = q;   // case 3: (n+m-1 th A) + q
    else                add = p_;  // case 4: (n+m-1 th A) + p  (capped)
    // expandLen(A,0): decrement the last element
    seq_[static_cast<size_t>(n - 1)] = an - 1;
    // append M elements, wrapping within the tail
    for (BigInt step = 1; step <= M; ++step) {
        BigInt pos1 = n + step - 1;                 // unified (n+m-1)
        BigInt off = ((pos1 - 1 - br1) % L + L) % L; // wrap into tail
        seq_.push_back(tail[static_cast<size_t>(off)] + add);
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
    if (an == 0) return *this;
    BigInt br1 = rightmostLess_(an);
    BigInt L = n - br1;
    if (m == 0) {
        if (an > 0) seq_[static_cast<size_t>(n - 1)] = an - 1;
        return *this;
    }
    // expand(A, m>0) = expandLen(A, m*L - 1)
    BigInt M = m * L - 1;
    return expandLen_(M);
}

// expand_to(len) maps to expandLen(A, len): decrement last, append `len` tail
// elements (with per-mode add).
EpspSS& EpspSS::expand_to(BigInt len) {
    return expandLen_(len);
}

int EpspSS::compare(const Notation& other) const {
    const EpspSS* o = dynamic_cast<const EpspSS*>(&other);
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

bool EpspSS::isSuccessor() const {
    return !seq_.empty() && seq_.back() == 1;
}

BigInt EpspSS::expandUntilLarger_(EpspSS& work, const std::vector<BigInt>& target,
                                   size_t startIdx) {
    BigInt origLen = static_cast<BigInt>(work.seq_.size());
    BigInt val = work.seq_.back();
    BigInt targetVal = target[startIdx];
    if (origLen == static_cast<BigInt>(target.size()) && val == targetVal) return 0;
    if (val <= targetVal) return -1; // smaller than target -> non-standard
    if (val > targetVal + 1) {
        work.seq_.back() = targetVal + 1; // clamp down, then expand one step
    }
    for (size_t i = static_cast<size_t>(origLen); i < target.size(); ++i) {
        work.expand(1);
        size_t last = work.seq_.size() - 1;
        if (work.seq_[last] > target[last]) return static_cast<BigInt>(work.seq_.size()) - origLen;
        if (work.seq_[last] < target[last]) return -1; // non-standard
    }
    return static_cast<BigInt>(work.seq_.size()) - origLen;
}

// Standard form (标准型), adapted from YSequence::checkStandardAndNonMaximum
// (YSequence.cpp). ⚠ canonical starter [0, seq_[1]] (vs YSequence's
// [1, seq[1]+1]) and exact EpspSS semantics need verification. The loop is
// stall-guarded so it always terminates; on a stall or non-standard input,
// *this is left unchanged.
void EpspSS::normalize() {
    size_t n = seq_.size();
    if (n == 0) return;
    if (n == 1) { seq_[0] = 0; return; }
    if (n == 2) {
        seq_[0] = 0;
        if (seq_[1] < 0) seq_[1] = 0;
        return;
    }
    EpspSS work(p_);
    work.seq_ = {0, seq_[1]};
    size_t idx = 0;
    const BigInt kGuard = 100000;
    BigInt steps = 0;
    while (idx < n) {
        size_t guard = 0;
        while (work.seq_.size() <= idx) {
            std::vector<BigInt> before = work.seq_;
            work.expand(1);
            if (work.seq_ == before) return;
            if (++guard > static_cast<size_t>(kGuard)) return;
        }
        if (work.seq_[idx] < seq_[idx]) return;
        if (work.seq_[idx] == seq_[idx]) { idx++; continue; }
        BigInt added = expandUntilLarger_(work, seq_, idx);
        if (added == -1) return;
        idx += static_cast<size_t>(added);
        if (++steps > kGuard) return;
    }
    seq_ = work.seq_;
}

std::istream& operator>>(std::istream& is, EpspSS& p) {
    std::string s;
    std::getline(is, s);
    if (!s.empty()) p.string_to_it(s);
    return is;
}

} // namespace ordinal
} // namespace googology
