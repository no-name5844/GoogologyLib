#include "googology/notations/ordinal/matrix/bms/BMS.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>

using namespace googology;
using namespace googology::ordinal;

// ---------------------------------------------------------------------------
// 存储访问
// ---------------------------------------------------------------------------
BigInt BMS::get_(int x, int y) const {
    if (x < 0 || x >= numCols_()) return 0;
    if (y < 0 || y >= colHeight_(x)) return 0;
    return cols_[x][y];
}

// ---------------------------------------------------------------------------
// §0.1 合法矩阵三条件（直接句法判定；合法 ≠ 标准型）
// ---------------------------------------------------------------------------
bool BMS::isLegal_() const {
    int X = numCols_();
    if (X == 0) return true;                       // 空矩阵视为合法
    // 1) 首列全 0
    int h0 = colHeight_(0);
    for (int y = 0; y < h0; ++y)
        if (get_(0, y) != 0) return false;
    // 2) 每列自上而下非增
    for (int x = 0; x < X; ++x) {
        int h = colHeight_(x);
        for (int y = 0; y + 1 < h; ++y)
            if (get_(x, y) < get_(x, y + 1)) return false;
    }
    // 3) 同行不超前超 1：S_{x,y} <= max_{p<x} S_{p,y} + 1
    for (int x = 1; x < X; ++x) {
        int h = colHeight_(x);
        for (int y = 0; y < h; ++y) {
            BigInt mx = 0;
            for (int p = 0; p < x; ++p)
                mx = std::max(mx, get_(p, y));
            if (get_(x, y) > mx + 1) return false;
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
// §0.2.1 共享辅助
// ---------------------------------------------------------------------------
int BMS::lnzRow_() const {
    int X = numCols_();
    if (X == 0) return -1;
    int h = colHeight_(X - 1);
    int z = -1;
    for (int y = 0; y < h; ++y)
        if (get_(X - 1, y) > 0) z = y;
    return z;
}

bool BMS::lastColAllZero_() const { return lnzRow_() < 0; }

int BMS::badRoot_() const {
    int z = lnzRow_();
    if (z < 0) return -1;
    return parentOf(z, numCols_() - 1);
}

BigInt BMS::delta_(int k) const {
    int z = lnzRow_();
    if (z < 0 || k >= z) return 0;
    int r = badRoot_();
    return get_(numCols_() - 1, k) - get_(r, k);
}

void BMS::copyColumn_(int m, int i, std::vector<BigInt>& out) const {
    int r = badRoot_();
    int orig = r + m;                       // 原列标
    int hh = colHeight_(orig);
    out.assign(hh, 0);
    for (int k = 0; k < hh; ++k) {
        BigInt a = ascensionDegree(k, orig, r) ? 1 : 0;
        out[k] = get_(orig, k) + a * delta_(k) * static_cast<BigInt>(i);
    }
}

// ---------------------------------------------------------------------------
// §0.2.2 expand 骨架：FS_n(S) = G + B^(0) + ... + B^(n-1)
// ---------------------------------------------------------------------------
void BMS::expandToFS_(BigInt n) {
    if (n < 1) n = 1;
    int X = numCols_();
    if (X == 0) {                                   // expand((),n)=n：n 个零列
        cols_.clear();
        for (BigInt i = 0; i < n; ++i) cols_.push_back({0});
        return;
    }
    if (lastColAllZero_()) {                         // 末列全 0：剥列后递归一次
        cols_.pop_back();
        expandToFS_(n);
        return;
    }
    int z = lnzRow_();
    int r = badRoot_();
    if (r < 0) {                                    // 末列无父项（如单列）：递减 LNZ
        for (BigInt i = 0; i < n; ++i) {
            if (numCols_() == 0) break;
            int zz = lnzRow_();
            if (zz < 0) break;                     // 无 LNZ 可递减
            cols_[numCols_() - 1][zz] -= 1;
            if (lnzRow_() < 0) cols_.pop_back();  // 归零则剥列
        }
        return;
    }
    // G = 列 0..r-1；B = 列 r..X-2
    std::vector<std::vector<BigInt>> G;
    if (r > 0) G.assign(cols_.begin(), cols_.begin() + r);
    int Bcols = (X - 1) - r;                    // 坏部列数
    std::vector<std::vector<BigInt>> result = G;
    for (BigInt i = 0; i < n; ++i) {            // B^(0) .. B^(n-1)
        for (int m = 0; m < Bcols; ++m) {
            std::vector<BigInt> col;
            copyColumn_(m, static_cast<int>(i), col);
            result.push_back(col);
        }
    }
    cols_ = std::move(result);
}

BMS& BMS::expand(BigInt n) {
    if (is_master_limit_) {                       // master_limit().expand(m) == limit(m)
        auto lm = limit(n);
        cols_.clear();
        const BMS* src = lm.get();
        for (int x = 0; x < src->numCols_(); ++x) {
            std::vector<BigInt> col;
            for (int y = 0; y < src->colHeight_(x); ++y)
                col.push_back(src->get_(x, y));
            cols_.push_back(col);
        }
        is_master_limit_ = false;
        return *this;
    }
    is_master_limit_ = false;
    expandToFS_(n);
    return *this;
}

// §0.3 极限表达式（所有 BMS 版本共用）：
//   limit(0)=(), limit(1)=(0), limit(2)=(0)(1),
//   limit(3)=(0)(1,1), ..., limit(n)=(0)(1,1,...,1) [n-1 个 1]
// master_limit() = (0)(1,1,1,...)（第二列全 1，无限），标记为极限表达式。
// 返回 shared_ptr<BMS>（动态类型恒为 BM4，极限表达式版本无关）。
std::shared_ptr<BMS> BMS::limit(BigInt n) {
    std::string s;
    if (n <= 0) s = "";                              // limit(0) = ()
    else {
        s = "(0)";
        if (n >= 2) {                                // 第二列：n-1 个 1
            s += "(";
            for (BigInt k = 0; k < n - 1; ++k) { if (k) s += ","; s += "1"; }
            s += ")";
        }
    }
    return std::make_shared<BM4>(s);
}

std::shared_ptr<BMS> BMS::master_limit() {
    auto r = std::make_shared<BM4>();
    r->is_master_limit_ = true;
    return r;
}

// ---------------------------------------------------------------------------
// 父项 / 上升度（virtual 钩子，BM4 默认）
// ---------------------------------------------------------------------------
int BMS::parentOf(int k, int m) const {
    int best = -1;
    for (int p = 0; p < m; ++p)
        if (get_(p, k) < get_(m, k)) {
            if (k == 0 || ascensionDegree(k - 1, m, p))
                best = p;
        }
    return best;                              // -1 表示无父项
}

bool BMS::ascensionDegree(int k, int m, int t) const {
    if (m == t) return true;                  // (p_k)^0(m) = m
    int cur = m;
    int guard = 0;
    while (guard++ < 100000) {
        int p = parentOf(k, cur);
        if (p < 0 || p == cur) break;       // 无更多祖先
        cur = p;
        if (cur == t) return true;
    }
    return false;
}

// BM1：父项去祖先检查；上升度按首行统一
int BM1::parentOf(int k, int m) const {
    int best = -1;
    for (int p = 0; p < m; ++p)
        if (get_(p, k) < get_(m, k)) best = p;
    return best;
}
bool BM1::ascensionDegree(int k, int m, int t) const {
    return BMS::ascensionDegree(0, m, t);   // 忽略 k>0，统一用首行
}

// BM3.3：上升度去掉 "父项 = 坏根" 子句。
// 笔记 §9.2.1 公式 a_{k,m}=1 ⇔ a_{k,parent(m)}=1 ∧ (parent(m) > r)。
// 即 P(m) = P(parent(m)) ∧ (parent(m) > t)，t 即坏根列（ascensionDegree
// 的第三个参数）。注意 t 由 copyColumn_ 直接传入，切勿在此再调 badRoot_()
// （否则 parentOf→ascensionDegree→badRoot_ 形成环，栈溢出）。
bool BM3_3::ascensionDegree(int k, int m, int t) const {
    if (m == t) return true;                  // (p_k)^0(m) = m
    int p = parentOf(k, m);
    if (p < 0 || p == m) return false;      // 无祖先 -> 不可达 t
    if (!ascensionDegree(k, p, t)) return false;   // P(parent(m))
    if (!(p > t)) return false;               // BM3.3：严格 > r(=t)，去掉 ==r
    return true;
}

// ---------------------------------------------------------------------------
// 比较（语法全序，列主序字典序；非真序数序）
// ---------------------------------------------------------------------------
int BMS::compare(const Notation& other) const {
    const BMS* o = dynamic_cast<const BMS*>(&other);
    if (!o) throw NotComparable(name());
    int X1 = numCols_(), X2 = o->numCols_();
    int X = std::max(X1, X2);
    for (int x = 0; x < X; ++x) {
        int h1 = colHeight_(x), h2 = o->colHeight_(x);
        int h = std::max(h1, h2);
        for (int y = 0; y < h; ++y) {
            BigInt a = get_(x, y), b = o->get_(x, y);
            if (a != b) return (a < b) ? -1 : 1;
        }
    }
    if (X1 != X2) return (X1 < X2) ? -1 : 1;
    return 0;
}

// ---------------------------------------------------------------------------
// 规范化 / 合法性 / 后继
// ---------------------------------------------------------------------------
void BMS::normalize() {
    if (is_master_limit_) return;
    if (isLegal_()) return;                    // 已合法 -> 无操作
    // 不满足§0.1：按库惯例保持 *this 不变
}

// 当前以 §0.1 合法性替代标准型检测（必要非充分）。BMS 无法用 §12 引擎
// 判定真标准型——compare 为句法序而非真序数序，Trans() 可能不终止。
bool BMS::is_standard() const { return isLegal_(); }

bool BMS::isSuccessor() const { return false; }   // 无简单后继概念

std::vector<std::shared_ptr<OrdinalNotation>> BMS::roots() const {
    // BMS 用 §0.1 合法性直接判定（不依赖 §12 引擎），此处仅作纯虚满足：
    // 返回与 *this 同动态类型的空矩阵种子。
    auto seed = std::shared_ptr<OrdinalNotation>(this->clone());
    static_cast<BMS*>(seed.get())->cols_.clear();
    std::vector<std::shared_ptr<OrdinalNotation>> r;
    r.push_back(seed);
    return r;
}

// ---------------------------------------------------------------------------
// 解析 / 序列化
// ---------------------------------------------------------------------------
void BMS::string_to_it(const std::string& s) {
    cols_.clear();
    std::string t = s;
    // 去掉空白
    t.erase(std::remove_if(t.begin(), t.end(), ::isspace), t.end());
    size_t i = 0, L = t.size();
    while (i < L && t[i] == '(') {
        size_t j = t.find(')', i);
        if (j == std::string::npos) break;
        std::string inner = t.substr(i + 1, j - i - 1);
        std::vector<BigInt> col;
        if (!inner.empty()) {
            std::stringstream ss(inner);
            std::string item;
            while (std::getline(ss, item, ',')) {
                if (!item.empty()) col.push_back(BigInt(std::stoll(item)));
            }
        }
        cols_.push_back(col);
        i = j + 1;
    }
}

std::string BMS::to_string() const {
    if (is_master_limit_) return "(0)(1,1,1,…)";   // 极限表达式（第二列全 1，无限）
    std::string out;
    for (size_t x = 0; x < cols_.size(); ++x) {
        out += '(';
        for (size_t y = 0; y < cols_[x].size(); ++y) {
            if (y) out += ',';
            out += std::to_string(cols_[x][y]);
        }
        out += ')';
    }
    return out;
}

Capabilities BMS::capabilities() const {
    Capabilities c;
    c.set(Op::FromString);
    c.set(Op::ToString);
    c.set(Op::Expand);
    c.set(Op::Compare);
    c.set(Op::Normalize);
    return c;
}

std::istream& operator>>(std::istream& is, BMS& b) {
    std::string s;
    std::getline(is, s);
    if (!s.empty()) b.string_to_it(s);
    return is;
}
