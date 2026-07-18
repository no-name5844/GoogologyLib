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

// 列「值高度」：去掉末尾连续 0 后的有效高度，但**至少保留 1**
// （非空列最低是零列 (0)，不是空列）。例：(0,0,0) → 1（= (0)），
// (2,2,0) → 2（= (2,2)），(1,1,1) → 3。尾随零不改变整体大小
// （用户 2026-07-18 明确：任意列末尾加 0 不改值），但展开须看满形
// （同用户：展开要看）。
int BMS::valHeight_(int x) const {
    int h = colHeight_(x);
    if (h == 0) return 0;                       // 真正空列（不应出现）
    while (h > 1 && get_(x, h - 1) == 0) --h; // 保留到高度 1，零列 = (0)
    return h;
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
// §0.2.2 expand 骨架（末列非全 0 的极限情形）：
//   用户 2026-07-18 公式（本会话重发确认「基本」= 基本列定义）：
//     expand((),n)    = n
//     expand(S+(0),n) = S
//     expand(S,n)      = G + B^(0)+...+B^(n)   (末列非全 0)
//   用户本会话明确「原本的 n=1,2 对应库的 n=0,1」（整体偏移 1）：
//     lib expand(0) = spec FS_1 = G + B^(0)       （1 块，原本 n=1）
//     lib expand(1) = spec FS_2 = G + B^(0)+B^(1) （2 块，原本 n=2）
//     lib expand(k) = spec FS_{k+1} = G + B^(0)+...+B^(k)（k+1 块）
//   故一步 expand(n) 的坏块数 = n+1（含上界 i<=n），即原公式 RHS。
//   该公式本身是「递归骨架」的一步；下一步再以 2n 递归（spec §0.2.2
//   第二式），库不替用户做那步递归。FS_n 随 n 严格小于 S、单调向 S
//   逼进，故可被 §12 引擎经有限次 expand 可达判定（与 spec n 偏移 1
//   仅是索引差，不改变单调性）。
// ---------------------------------------------------------------------------
void BMS::expandToFS_(BigInt n) {
    int X = numCols_();
    if (X == 0) {                                   // expand((),n)=n：n 个零列
        cols_.clear();
        return;
    }
    if (lastColAllZero_()) {   // 末列全 0（§0.1.3 无坏根 = 后继矩阵）：
        // 一步基本列 FS_n = 前驱 = 去掉末列，**不再递归/求值**。库约定
        // expand(n) 只产出一步 FS_n（见 spec §0.2.2 库约定）；后继的一步
        // 展开就是其前驱，与 Prss 后继剥末位（seq.pop_back）语义一致。
        // 旧实现「剥列后再 expandToFS_」执行的是 spec 的完整递归求值
        // expand(S+(0),n)=expand(S,2n)，会把 (0)(0)…(0) 变成不动点，令
        // §12 引擎无法下降到 () 及更小后继——已修正。
        cols_.pop_back();
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
    // 坏块数 = n+1（含上界 i<=n）：lib expand(k) = spec FS_{k+1}
    //   = G + B^(0)+...+B^(k)。k=0 → 1 块 = 好部 G + 坏部 B^(0)
    //   （用户「n=0 该给好+坏」）；k>=1 → k+1 块 = spec FS_{k+1}。
    //   整体偏移 1（原本 n=1,2 ↔ lib n=0,1）。FS_k 随 k 严格小于
    //   S、单调向 S 逼进，可被 §12 引擎经有限次 expand 可达判定。
    BigInt nb = n + 1;
    for (BigInt i = 0; i < nb; ++i) {            // B^(0) .. B^(n)
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
    // 极限表达式（master_limit）= 上确界，大于一切有限矩阵。§12 引擎用
    // compare 做剪枝，必须让根（master_limit）比目标大，否则引擎恒返回 false。
    if (is_master_limit_ || o->is_master_limit_) {
        if (is_master_limit_ && o->is_master_limit_) return 0;
        return is_master_limit_ ? 1 : -1;
    }
    // 值比较：每列先去掉末尾连续 0（尾随零不改大小，用户 2026-07-18
    // 明确），再列主序字典序。列数不同且共享列均相等时，列多者更大。
    int X1 = numCols_(), X2 = o->numCols_();
    int X = std::max(X1, X2);
    for (int x = 0; x < X; ++x) {
        int h1 = valHeight_(x), h2 = o->valHeight_(x);
        int h = std::max(h1, h2);
        for (int y = 0; y < h; ++y) {
            BigInt a = (y < h1) ? get_(x, y) : 0;
            BigInt b = (y < h2) ? o->get_(x, y) : 0;
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

// 标准型检测：BMS 不覆写 is_standard()——它有基本列（expand）与极限表达式
// （master_limit），故直接继承 OrdinalNotation 的通用 §12 引擎，与 Prss /
// ε_pSS / ε_ωSS / WeakVeblen 等**完全一致，无任何区别**。§12 可达性只需
// 「基本列 + 极限表达式」，不需要真序数序；BMS 的句法列序在标准型上本就与
// 序数序一致，足够供引擎剪枝。isLegal_() 只表达 §0.1 合法性（合法 ≠ 标准型），
// 供 normalize / 解析用，不再充当标准型检测。

bool BMS::isSuccessor() const { return false; }   // 无简单后继概念

std::vector<std::shared_ptr<OrdinalNotation>> BMS::roots() const {
    // §12 引擎的根 = 本记号系统的极限表达式。BMS 各版本共用同一极限表达式
    // master_limit() = (0)(1,1,1,…)，其 expand(n) = limit(n)。
    std::vector<std::shared_ptr<OrdinalNotation>> r;
    r.push_back(master_limit());
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
    // 输入「补全」（用户 2026-07-18）：解析后把各列补齐到最大列高
    // （末尾补 0），使矩阵成矩形满形。展开须看满形（展开要看），而
    // 比较/输出用值/简写。列数不变、只补列内尾随零 = 值不变。
    int maxH = 0;
    for (auto& c : cols_) maxH = std::max(maxH, static_cast<int>(c.size()));
    for (auto& c : cols_) c.resize(maxH, 0);
}

std::string BMS::to_string() const {
    if (is_master_limit_) return "(0)(1,1,1,…)";   // 极限表达式（第二列全 1，无限）
    // 输出用简写（用户 2026-07-18）：每列去掉末尾连续 0。
    std::string out;
    for (size_t x = 0; x < cols_.size(); ++x) {
        int h = valHeight_(static_cast<int>(x));
        out += '(';
        for (int y = 0; y < h; ++y) {
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
