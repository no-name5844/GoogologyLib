#include "googology/notations/ordinal/sequence/marked_parent/pps/PPS.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <typeinfo>

namespace googology {
namespace ordinal {

Capabilities Pps::capabilities() const {
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

std::string Pps::name() const {
    switch (variant_) {
        case Variant::PPS1:  return "pps1";
        case Variant::PPS2:  return "pps2";
        case Variant::PPS4:  return "pps4";
        case Variant::wPPS4: return "wpps4";
        case Variant::tPPS4: return "tpps4";
        case Variant::fPPS4: return "fpps4";
    }
    return "pps";
}

std::string Pps::version() const {
    switch (variant_) {
        case Variant::PPS1:  return "1";
        case Variant::PPS2:  return "2";
        case Variant::PPS4:  return "4";
        case Variant::wPPS4: return "4w";
        case Variant::tPPS4: return "4t";
        case Variant::fPPS4: return "4f";
    }
    return "?";
}

// 开区间 (lo, hi) 列标内最右侧值 = b 的列标（1-based）；无则 0。
// 列标越界（< 1 或 > 序列长）视为不存在。
BigInt Pps::rightmostEq_(BigInt lo, BigInt hi, BigInt b) const {
    BigInt y = static_cast<BigInt>(seq_.size());
    for (BigInt j = hi - 1; j > lo; --j) {
        if (j < 1 || j > y) continue;
        if (seq_[static_cast<size_t>(j - 1)] == b) return j;
    }
    return 0;
}

// 字典序比较 a_{k+j} 与 a_{c+j}（j = 0,1,2,...）：首个不同处前者大 → 1，
// 前者小 → -1；一侧越界则短者小；两侧同时越界 → 0（全等平局）。
int Pps::tailCmp_(BigInt k, BigInt c) const {
    BigInt y = static_cast<BigInt>(seq_.size());
    for (BigInt j = 0;; ++j) {
        BigInt ik = k + j, ic = c + j;
        bool okK = (ik >= 1 && ik <= y);
        bool okC = (ic >= 1 && ic <= y);
        if (!okK && !okC) return 0;
        if (!okK) return -1;
        if (!okC) return 1;
        if (seq_[static_cast<size_t>(ik - 1)] != seq_[static_cast<size_t>(ic - 1)])
            return (seq_[static_cast<size_t>(ik - 1)] > seq_[static_cast<size_t>(ic - 1)]) ? 1 : -1;
    }
}

// 按变体替换末项：返回新末项值；strong 表示进入强展开分支（PPS4 系）。
BigInt Pps::replaceLast_(BigInt x, BigInt b, BigInt y, bool& strong) const {
    strong = false;
    switch (variant_) {
        case Variant::PPS1: {
            // 坏根与末项之间存在值 = b 的项 → 换为 b；否则减 1。
            BigInt k = rightmostEq_(x, y, b);
            return (k != 0) ? b : x - 1;
        }
        case Variant::PPS2: {
            // 存在值 = b 的项（取最右，列标 k）时，a_{k+j} 与 a_{c+j}
            // （c = x）首个不同处前者更大才换为 b，否则减 1。
            BigInt k = rightmostEq_(x, y, b);
            if (k != 0 && tailCmp_(k, x) > 0) return b;
            return x - 1;
        }
        case Variant::PPS4:
        case Variant::wPPS4:
        case Variant::tPPS4:
        case Variant::fPPS4: {
            // 弱展开：坏根与末项之间存在值 = b 的项 → 换为 b。
            BigInt k = rightmostEq_(x, y, b);
            if (k != 0) return b;
            // 强展开：第 b 列与第 x 列之间（不含）最右侧值 = b 的项，
            // 末项换为该列标；找不到则等同弱展开（换为 b）。
            strong = true;
            BigInt j = rightmostEq_(b, x, b);
            return (j != 0) ? j : b;
        }
    }
    return x - 1; // unreachable
}

// 复制规则：追加项源位置 i（1-based）处值 src 的映射。
// 公共规则：src >= x → src + L；否则原样。tPPS4/fPPS4 在强展开分支对
// 末项链（源 i = y, y+L, y+2L, ...）加阶差 L；fPPS4 另将末项第一次复制
// （源 i = y）定为坏根列标 x。
BigInt Pps::copied_(BigInt src, BigInt x, BigInt L, BigInt y, BigInt i, bool strong) const {
    BigInt base = (src >= x) ? src + L : src;
    if (!strong) return base;
    if (variant_ == Variant::tPPS4 || variant_ == Variant::fPPS4) {
        if (i % L == y % L) {   // 末项链
            if (variant_ == Variant::fPPS4 && i == y) return x;
            return src + L;
        }
    }
    return base;
}

// expandLen(A, M)：替换末项 + 追加 M 项（第 p 项由源 i = p + y - L 确定）。
Pps& Pps::expandLen_(BigInt M) {
    if (seq_.empty()) return *this;
    BigInt y = static_cast<BigInt>(seq_.size());
    BigInt x = seq_.back();               // 末项值
    if (x == 0) { seq_.pop_back(); return *this; }   // 后继：去掉末项
    if (x >= y) throw std::out_of_range(
        "Pps::expandLen_: 坏根 = 第 x 项不存在（x >= y）");
    BigInt b = seq_[static_cast<size_t>(x - 1)];   // 坏根值
    BigInt L = y - x;                              // 尾长，>= 1
    bool strong = false;
    seq_[static_cast<size_t>(y - 1)] = replaceLast_(x, b, y, strong);
    for (BigInt p = 1; p <= M; ++p) {
        BigInt i = p + y - L;                      // 源位置（1-based），> x
        BigInt src = seq_[static_cast<size_t>(i - 1)];
        seq_.push_back(copied_(src, x, L, y, i, strong));
    }
    return *this;
}

void Pps::string_to_it(const std::string& s) {
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

std::string Pps::to_string() const {
    std::ostringstream os;
    os << "(";
    for (size_t i = 0; i < seq_.size(); ++i) {
        if (i) os << ", ";
        os << seq_[i];
    }
    os << ")";
    return os.str();
}

// 库 0 索引：expand(k) = 基本列[k+1] = expandLen((k+1)L - 1)。
Pps& Pps::expand(BigInt n) {
    if (is_master_limit_) { fillLimit_(n); return *this; }
    if (seq_.empty()) return *this;
    BigInt y = static_cast<BigInt>(seq_.size());
    BigInt x = seq_.back();
    if (x == 0) { seq_.pop_back(); return *this; }
    if (x >= y) throw std::out_of_range(
        "Pps::expand: 坏根 = 第 x 项不存在（x >= y）");
    BigInt L = y - x;
    return expandLen_((n + 1) * L - 1);
}

Pps& Pps::expand_to(BigInt len) {
    return expandLen_(len);
}

// 字典序序数比较（标准型下），同 PrSS。仅同变体可比；主极限表达式为上确界。
int Pps::compare(const Notation& other) const {
    if (typeid(*this) != typeid(other)) throw NotComparable(name());
    const Pps& o = static_cast<const Pps&>(other);
    if (is_master_limit_ || o.is_master_limit_) {
        if (is_master_limit_ && o.is_master_limit_) return 0;
        return is_master_limit_ ? 1 : -1;
    }
    size_t n1 = seq_.size(), n2 = o.seq_.size();
    size_t m = std::min(n1, n2);
    for (size_t i = 0; i < m; ++i) {
        if (seq_[i] > o.seq_[i]) return 1;
        if (seq_[i] < o.seq_[i]) return -1;
    }
    if (n1 > n2) return 1;
    if (n1 < n2) return -1;
    return 0;
}

// 填入极限式 limit(n) = (0,1,...,n-1)（并清除主极限标记）。
void Pps::fillLimit_(BigInt n) {
    seq_.clear();
    for (BigInt k = 0; k < n; ++k) seq_.push_back(k);
    is_master_limit_ = false;
}

// --- 各变体样板：limit / master_limit / operator[] / roots ---

Pps1 Pps1::limit(BigInt n) { Pps1 r; r.fillLimit_(n); return r; }
Pps1 Pps1::master_limit() { Pps1 r; r.is_master_limit_ = true; return r; }
Pps1 Pps1::operator[](BigInt n) const { return idx_(*this, n); }
std::vector<std::shared_ptr<OrdinalNotation>> Pps1::roots() const {
    std::vector<std::shared_ptr<OrdinalNotation>> r;
    r.push_back(std::shared_ptr<OrdinalNotation>(new Pps1(master_limit())));
    return r;
}

Pps2 Pps2::limit(BigInt n) { Pps2 r; r.fillLimit_(n); return r; }
Pps2 Pps2::master_limit() { Pps2 r; r.is_master_limit_ = true; return r; }
Pps2 Pps2::operator[](BigInt n) const { return idx_(*this, n); }
std::vector<std::shared_ptr<OrdinalNotation>> Pps2::roots() const {
    std::vector<std::shared_ptr<OrdinalNotation>> r;
    r.push_back(std::shared_ptr<OrdinalNotation>(new Pps2(master_limit())));
    return r;
}

Pps4 Pps4::limit(BigInt n) { Pps4 r; r.fillLimit_(n); return r; }
Pps4 Pps4::master_limit() { Pps4 r; r.is_master_limit_ = true; return r; }
Pps4 Pps4::operator[](BigInt n) const { return idx_(*this, n); }
std::vector<std::shared_ptr<OrdinalNotation>> Pps4::roots() const {
    std::vector<std::shared_ptr<OrdinalNotation>> r;
    r.push_back(std::shared_ptr<OrdinalNotation>(new Pps4(master_limit())));
    return r;
}

WPps4 WPps4::limit(BigInt n) { WPps4 r; r.fillLimit_(n); return r; }
WPps4 WPps4::master_limit() { WPps4 r; r.is_master_limit_ = true; return r; }
WPps4 WPps4::operator[](BigInt n) const { return idx_(*this, n); }
std::vector<std::shared_ptr<OrdinalNotation>> WPps4::roots() const {
    std::vector<std::shared_ptr<OrdinalNotation>> r;
    r.push_back(std::shared_ptr<OrdinalNotation>(new WPps4(master_limit())));
    return r;
}

TPps4 TPps4::limit(BigInt n) { TPps4 r; r.fillLimit_(n); return r; }
TPps4 TPps4::master_limit() { TPps4 r; r.is_master_limit_ = true; return r; }
TPps4 TPps4::operator[](BigInt n) const { return idx_(*this, n); }
std::vector<std::shared_ptr<OrdinalNotation>> TPps4::roots() const {
    std::vector<std::shared_ptr<OrdinalNotation>> r;
    r.push_back(std::shared_ptr<OrdinalNotation>(new TPps4(master_limit())));
    return r;
}

FPps4 FPps4::limit(BigInt n) { FPps4 r; r.fillLimit_(n); return r; }
FPps4 FPps4::master_limit() { FPps4 r; r.is_master_limit_ = true; return r; }
FPps4 FPps4::operator[](BigInt n) const { return idx_(*this, n); }
std::vector<std::shared_ptr<OrdinalNotation>> FPps4::roots() const {
    std::vector<std::shared_ptr<OrdinalNotation>> r;
    r.push_back(std::shared_ptr<OrdinalNotation>(new FPps4(master_limit())));
    return r;
}

} // namespace ordinal
} // namespace googology
