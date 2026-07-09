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

// expandLen(A, M). Unified tail-wrap: appended index is (n+m-1) (1-based,
// wraps cyclically in the tail [br+1..n]); the added amount is 0 for
// q==1 (case 2) and q = a_n - a_{br} otherwise (unbounded, no case 4).
// See spec/notations/epsilon_omega_ss.md (⚠ closure).
EpsOmegaSS& EpsOmegaSS::expandLen_(BigInt M) {
    if (seq_.empty()) return *this;
    if (seq_.back() == 1) { seq_.pop_back(); return *this; }
    if (M <= 0) {
        if (seq_.back() > 0) seq_.back() -= 1; // expandLen(A,0)
        return *this;
    }
    BigInt n = static_cast<BigInt>(seq_.size());
    BigInt an = seq_.back();
    if (an == 0) return *this;                  // ⚠ degenerate all-zero guard
    BigInt br1 = rightmostLess_(an);
    BigInt L = n - br1;
    std::vector<BigInt> tail(static_cast<size_t>(L));
    for (BigInt i = 0; i < L; ++i)
        tail[static_cast<size_t>(i)] = seq_[static_cast<size_t>(br1 + i)];
    BigInt q = an - seq_[static_cast<size_t>(br1 - 1)];
    int add = (q == 1) ? 0 : static_cast<int>(q); // case 2: +0; else +q (unbounded)
    seq_[static_cast<size_t>(n - 1)] = an - 1;  // expandLen(A,0): decrement last
    for (BigInt step = 1; step <= M; ++step) {
        BigInt pos1 = n + step - 1;                  // unified (n+m-1)
        BigInt off = ((pos1 - 1 - br1) % L + L) % L;
        seq_.push_back(tail[static_cast<size_t>(off)] + add);
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
    if (seq_.back() == 1) { seq_.pop_back(); return *this; }
    BigInt n = static_cast<BigInt>(seq_.size());
    BigInt an = seq_.back();
    if (an == 0) return *this;
    BigInt br1 = rightmostLess_(an);
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

bool EpsOmegaSS::isSuccessor() const {
    return !seq_.empty() && seq_.back() == 1;
}

BigInt EpsOmegaSS::expandUntilLarger_(EpsOmegaSS& work, const std::vector<BigInt>& target,
                                       size_t startIdx) {
    BigInt origLen = static_cast<BigInt>(work.seq_.size());
    BigInt val = work.seq_.back();
    BigInt targetVal = target[startIdx];
    if (origLen == static_cast<BigInt>(target.size()) && val == targetVal) return 0;
    if (val <= targetVal) return -1;
    if (val > targetVal + 1) {
        work.seq_.back() = targetVal + 1;
    }
    for (size_t i = static_cast<size_t>(origLen); i < target.size(); ++i) {
        work.expand(1);
        size_t last = work.seq_.size() - 1;
        if (work.seq_[last] > target[last]) return static_cast<BigInt>(work.seq_.size()) - origLen;
        if (work.seq_[last] < target[last]) return -1;
    }
    return static_cast<BigInt>(work.seq_.size()) - origLen;
}

void EpsOmegaSS::normalize() {
    size_t n = seq_.size();
    if (n == 0) return;
    if (n == 1) { seq_[0] = 0; return; }
    if (n == 2) {
        seq_[0] = 0;
        if (seq_[1] < 0) seq_[1] = 0;
        return;
    }
    EpsOmegaSS work;
    work.seq_ = {0, seq_[1]};   // ⚠ canonical starter [0, a_2] to verify
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

std::istream& operator>>(std::istream& is, EpsOmegaSS& p) {
    std::string s;
    std::getline(is, s);
    if (!s.empty()) p.string_to_it(s);
    return is;
}

} // namespace ordinal
} // namespace googology
