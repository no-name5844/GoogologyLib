#pragma once
#include <memory>
#include <string>
#include <vector>
#include "googology/core/OrdinalNotation.hpp"
#include "googology/core/Capability.hpp"

namespace googology {
namespace ordinal {

// ===========================================================================
// BMS (Bashicu Matrix System) — 序数记号（矩阵 -> 序数，见笔记 §0 的
// Trans() 映射）。同一套展开骨架，差异只在 父项条件 与 上升度 a_{k,m}。
//
// 本库"一律不求值、只展开一步"，故**终止性/良基性被刻意忽略**。
// expand(n) 只产出第 n 项基本列
// FS_n(S) = G + B^(0) + ... + B^(n-1)（笔记"基本列前驱"定义，有限可算）。
//
// 架构：基类 BMS 实现 §0 整体框架（矩阵存储 + §0.1 合法矩阵
// 三条件 + §0.2.1 共享辅助 + §0.2.2 expand 骨架），通过两个 virtual
// 钩子暴露"差异部分"：
//   * parentOf(k,m)      —— 父项 p_k(m)（含/不含上行祖先检查）
//   * ascensionDegree(k,m,t) —— 上升度 a_{k,m}(t)
// 各版本（BM1/BM4/BM3.3...）作为子类 override 这两个钩子。
//
// 存储：列主序。cols_[x] 是第 x 列（自上而下）。S_{x,y} = get(x,y)；
// 访问越界行按 0 处理（列可不等高，短列下方视为 0）。
// ===========================================================================
class BMS : public OrdinalNotation {
protected:
    // 列主序矩阵。
    std::vector<std::vector<BigInt>> cols_;

    // --- §0.1 合法矩阵三条件（直接句法判定，不依赖展开/终止）---
    // 1) 首列全 0；2) 每列自上而下非增；3) 同行不超前超 1。
    // 注：此为「合法矩阵」定义（不一定标准型），非标准型判定。
    bool isLegal_() const;

    // --- §0.2.1 共享辅助（皆用 virtual parentOf / ascensionDegree）---
    int numCols_() const { return static_cast<int>(cols_.size()); }
    int colHeight_(int x) const { return (x < 0 || x >= numCols_()) ? 0
                                         : static_cast<int>(cols_[x].size()); }
    // 列「值高度」：去掉末尾连续 0 后的有效高度（尾随零不改值），
    // 用于 compare / to_string；expand 仍用满形 colHeight_。
    int valHeight_(int x) const;
    // S_{x,y}；越界行按 0。
    BigInt get_(int x, int y) const;
    // 末列 LNZ 行 z = max{y | 末列[y] > 0}；末列全 0 返回 -1。
    int lnzRow_() const;
    // 末列是否全 0（无 LNZ、无坏根）。
    bool lastColAllZero_() const;
    // 坏根 r = parentOf(z, X-1)。要求 lnzRow_() >= 0。
    int badRoot_() const;
    // 阶差 Δ_k = S_{X-1,k} - S_{r,k}（k<z），k>=z 为 0。
    BigInt delta_(int k) const;
    // 把第 m 个坏部列（原列标 r+m）的第 i 个副本第 k 行写入 out。
    // B^(i)_{m,k} = B_{m,k} + a_{k,r+m}(r) * Δ_k * i。
    void copyColumn_(int m, int i, std::vector<BigInt>& out) const;

    // --- §0.2.2 expand 骨架（写入 cols_）---
    // FS_n(S) = G + B^(0) + ... + B^(n-1)（n>=1）。末列全 0 时
    // 剥掉该零列后递归一次（对应 expand(S+(0),n)=expand(S,2n) 的序+1 语义）。
    void expandToFS_(BigInt n);

public:
    BMS() = default;
    explicit BMS(const std::string& s) { string_to_it(s); }

    // 子类各自命名（"bm4"/"bm1"/"bm3.3"...）；基类抽象。
    std::string name() const override = 0;
    Family family() const override { return Family::Ordinal; }
    std::string subfamily() const override { return "bms"; }
    std::string style() const override { return "matrix"; }

    Capabilities capabilities() const override;

    void string_to_it(const std::string& s) override;
    std::string to_string() const override;

    // expand(n) = 第 n 项基本列 FS_n(S)，原地改写并返回 *this（协变返回）。
    BMS& expand(BigInt n) override;

    // 列主序字典序全序：用于容器/排序，也充当 §12 引擎的剪枝序。
    // 在**标准型**上它与真序数序一致（故足够供 is_standard 引擎使用）；
    // 我们不显式计算 Trans() 的真序数值（可能不终止——已丢进垃圾桶）。
    // master_limit（极限表达式）视为上确界。跨类型抛 NotComparable。
    int compare(const Notation& other) const override;

    // 规范化：强制 §0.1 合法性。若已满足则无操作；若不满足（非法矩阵）按库惯例
    // 保持 *this 不变。
    void normalize() override;

    // 标准型检测：BMS **不覆写** is_standard()。它有基本列（expand）与极限
    // 表达式（master_limit），故直接继承 OrdinalNotation 的通用 §12 引擎，与
    // 所有有基本列定义的序数记号（Prss / ε_pSS / ε_ωSS / WeakVeblen …）
    // **完全一致，不存在任何区别**。（isLegal_() 仅表达 §0.1 合法性，合法 ≠
    // 标准型，供 normalize / 解析用。）

    // BMS 无简单后继概念 -> false（文档说明）。基类 isSuccessor() 非 virtual，
    // 故此处不写 override。
    bool isSuccessor() const;

    // 多态拷贝（各子类实现）；roots() 返回本记号系统的极限表达式
    // master_limit()（§12 引擎的根）。
    OrdinalNotation* clone() const override = 0;
    std::vector<std::shared_ptr<OrdinalNotation>> roots() const override;

    // ===== 极限表达式（§0.3，所有 BMS 版本共用）=====
    //   limit(0)=(), limit(1)=(0), limit(2)=(0)(1),
    //   limit(3)=(0)(1,1), ..., limit(n)=(0)(1,1,...,1) [n-1 个 1]
    //   master_limit() = (0)(1,1,1,...)（第二列全 1，无限），标记为极限表达式。
    // master_limit().expand(m) == limit(m)（与 §12 约定一致）。
    // 返回 shared_ptr<BMS>（动态类型恒为 BM4，因极限表达式版本无关）。
    static std::shared_ptr<BMS> limit(BigInt n);
    static std::shared_ptr<BMS> master_limit();

    // ===== 版本差异钩子（§0.2.1 父项 / 上升度）=====
    // 父项 p_k(m)：最大 p<m 满足 S_{p,k} < S_{m,k}，且
    //   (k==0 ? true : ascensionDegree(k-1, m, p) == 1)。
    // BM4 默认实现；BM1 去祖先检查；其余版本按各自规则 override。
    virtual int parentOf(int k, int m) const;

    // 上升度 a_{k,m}(t)：t 是 m 在 p_k 下的祖先（含 m 自身）时为真。
    // BM4 默认 = ∃c≥0:(p_k)^c(m)=t。BM1 按首行统一；BM3.3 改子句。
    virtual bool ascensionDegree(int k, int m, int t) const;

    friend std::istream& operator>>(std::istream& is, BMS& b);
};

// ---------------------------------------------------------------------------
// BM4 (= BM2.3，现行标准，2018-09-01)。展开规则即 §0 骨架（父项含上行
// 祖先检查；上升度按各行独立判定祖先链）。是**唯一完整给出规则**的版本。
// ---------------------------------------------------------------------------
class BM4 : public BMS {
public:
    BM4() = default;
    explicit BM4(const std::string& s) : BMS(s) {}
    std::string name() const override { return "bm4"; }
    OrdinalNotation* clone() const override { return new BM4(*this); }
};

// ---------------------------------------------------------------------------
// BM1（初版，2014）。父项**缺上行祖先检查**（a_{k-1,m}(p)=1 不生效，
// 退化为仅按首行判定）；上升度由首行统一决定 a_{k,m}(t)=a_{0,m}(t)。
// 终止性：否（社区反例）。
// ---------------------------------------------------------------------------
class BM1 : public BMS {
public:
    BM1() = default;
    explicit BM1(const std::string& s) : BMS(s) {}
    std::string name() const override { return "bm1"; }
    OrdinalNotation* clone() const override { return new BM1(*this); }

    int parentOf(int k, int m) const override;
    bool ascensionDegree(int k, int m, int t) const override;
};

// ---------------------------------------------------------------------------
// BM3.3（Rpakr + ecl1psed，2019-03）。与 BM4 唯一差异在上升度：去掉
// "父项 = 坏根" 子句。示例（笔记 §9.2.2）：
//   (0,0,0)(1,1,1)(2,1,0)(1,1,1)
//   展开为 (0,0,0)(1,1,1)(2,1,0)(1,1,0)(2,2,1)(3,1,0)...
// 终止性：未证明。
// ---------------------------------------------------------------------------
class BM3_3 : public BMS {
public:
    BM3_3() = default;
    explicit BM3_3(const std::string& s) : BMS(s) {}
    std::string name() const override { return "bm3.3"; }
    OrdinalNotation* clone() const override { return new BM3_3(*this); }

    bool ascensionDegree(int k, int m, int t) const override;
};

// ---------------------------------------------------------------------------
// 未实现版本（笔记标注其 a_{k,m} 精确规则"仅见于 basmat 源码、公开文档未给"）：
//   BM2, BM2.1, BM2.2, BM3, BM3.1, BM3.2, PsiCubed2, Idealized 系列。
// 待获得权威公式后再以子类接入（同 parentOf/ascensionDegree 钩子）。
// ---------------------------------------------------------------------------

} // namespace ordinal
} // namespace googology
