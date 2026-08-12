#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "googology/core/OrdinalNotation.hpp"
#include "googology/core/Capability.hpp"

namespace googology {
namespace ordinal {

// ============================================================================
// PPS (Parented Predecessor Sequence) 系列 — 标记父项型 (marked_parent)
//
// 由 318`4 创造。PPS 系列 = PPS1（原始 PPS）+ PPS2 + PPS4 + 变体
// wPPS4 / tPPS4 / fPPS4。PPS3 / ePPS4 / sPPS4 资料缺失（N/A），不实现。
// 已知结论（外部）：良序极限 = ζ₀；PPS2 为失败的弱化修复；PPS4 旨在
// 删除无穷降链。本库一律不求值，expand 只产出一步基本列。
//
// 序列 A = (a_1, ..., a_n)，首项是第 1 项；极限表达式 0,1,2,3,4,5,...
//
// 公共骨架（全部变体一致）：
//   记末项值 x = a_n，末项列标 y = n，坏根 = 第 x 项，坏根值 b = a_x，
//   L = y - x。
//   1. 末项为 0 → 后继序数（一步展开 = 去掉末项）。
//   2. 按变体规则替换末项（见下）。
//   3. 复制：对任意 i > x，第 i+L 项由第 i 项确定：
//        a_i >= x → a_{i+L} = a_i + L；否则 a_{i+L} = a_i。
//   基本列[n] = 展开到第 y + nL - 1 项。库 0 索引（所有记号统一）：
//   expand(k) = 基本列[k+1] = expandLen((k+1)L - 1)（先替换末项，
//   再追加 (k+1)L - 1 项）。expand_to(M) = expandLen(A, M)。
//
// 变体差异（仅末项替换 / 强展开复制）：
//   PPS1 末项：坏根与末项之间存在值 = b 的项 → 换为 b；否则减 1。
//   PPS2 末项：存在值 = b 的项（取最右，列标 k）时，逐项比较 a_{k+j}
//     与 a_{c+j}（c = 坏根列标 = x，j = 0,1,2,...）：首个不同处前者更大
//     才换为 b，否则（更小 / 越界平局）减 1。
//   PPS4 末项：存在值 = b 的项 → 弱展开（换为 b）；否则强展开：在第 b 列
//     与第 x 列之间（都不含）找最右侧值 = b 的项，末项换为该列标，找不
//     到则等同弱展开（换为 b）。
//   wPPS4：注记称把 PPS4 的强展开判定由 ≤ b 收紧为 = b；PPS4 正文即
//     = b，故按字面实现后二者强展开行为一致（差异来源见 spec）。
//   tPPS4：wPPS4 + 强展开时由末项复制出的项加阶差（阶差 = L）。
//   fPPS4：tPPS4 + 强展开时末项第一次复制为坏根列标 x。
// ============================================================================

class Pps : public OrdinalNotation {
protected:
    enum class Variant { PPS1, PPS2, PPS4, wPPS4, tPPS4, fPPS4 };
    Variant variant_ = Variant::PPS1;

    explicit Pps(Variant v) : variant_(v) { baseVal_ = 0; }

    // 坏根与末项之间（开区间列标 (lo, hi)，两边都不含）最右侧值 = b 的
    // 列标；无则返回 0。列标越界（> 序列长 / < 1）视为无。
    BigInt rightmostEq_(BigInt lo, BigInt hi, BigInt b) const;
    // 字典序比较 a_{k+j} 与 a_{c+j}（j = 0,1,2,...，平局继续）：首个
    // 不同处前者大返回 1、前者小返回 -1；一侧先越界则短者小；同时越界
    // 返回 0（全等平局）。
    int tailCmp_(BigInt k, BigInt c) const;
    // 按变体替换末项。x = 末项原值，b = 坏根值，y = 末项列标。返回新末项值。
    BigInt replaceLast_(BigInt x, BigInt b, BigInt y, bool& strong) const;
    // 复制规则：追加项源位置 i（1-based）处值 src 的映射。
    // x = 末项原值，L = y - x，strong = 本次展开处于强展开分支。
    BigInt copied_(BigInt src, BigInt x, BigInt L, BigInt y, BigInt i, bool strong) const;
    // expandLen(A, M)：替换末项 + 追加 M 项。Mutates *this。
    Pps& expandLen_(BigInt M);

    // 按变体构造无标记极限式 limit(n) = (0,1,...,n-1)（填入本对象）。
    void fillLimit_(BigInt n);
    // A[n] 的非修改版实现：拷贝 + expand(n)。
    template <class Self>
    static Self idx_(const Self& s, BigInt n) {
        Self t = s;
        t.expand(n);
        return t;
    }

public:
    std::string name() const override;
    Family family() const override { return Family::Ordinal; }
    std::string subfamily() const override { return "sequence"; }
    std::string style() const override { return "marked_parent"; }

    // 创造者 / creator: 318`4 —— PPS 由 318`4 创造（见 spec pps.md）。
    // 版本 / version: 变体名（1 / 2 / 4 / 4w / 4t / 4f）。
    std::string creator() const override { return "318`4"; }
    std::string version() const override;

    Capabilities capabilities() const override;

    void string_to_it(const std::string& s) override;
    std::string to_string() const override;

    // 这些返回记号自身类型（重写的 Pps），而非字符串。
    Pps& expand(BigInt n) override;
    Pps& expand_to(BigInt len) override;   // -> expandLen(A, len)

    // 字典序序数比较（标准型下），同 PrSS。跨变体比较抛 NotComparable。
    // 主极限表达式比较为全体上确界。
    int compare(const Notation& other) const override;
};

// --- 各变体具体类：仅设定 variant，逻辑全在基类 ---

class Pps1 : public Pps {
public:
    Pps1() : Pps(Variant::PPS1) {}
    explicit Pps1(const std::string& s) : Pps(Variant::PPS1) { string_to_it(s); }
    static Pps1 limit(BigInt n);
    static Pps1 master_limit();
    Pps1 operator[](BigInt n) const;   // A[n] = expand(A,n)，不改变 *this
    OrdinalNotation* clone() const override { return new Pps1(*this); }
    std::vector<std::shared_ptr<OrdinalNotation>> roots() const override;
};

class Pps2 : public Pps {
public:
    Pps2() : Pps(Variant::PPS2) {}
    explicit Pps2(const std::string& s) : Pps(Variant::PPS2) { string_to_it(s); }
    static Pps2 limit(BigInt n);
    static Pps2 master_limit();
    Pps2 operator[](BigInt n) const;   // A[n] = expand(A,n)，不改变 *this
    OrdinalNotation* clone() const override { return new Pps2(*this); }
    std::vector<std::shared_ptr<OrdinalNotation>> roots() const override;
};

class Pps4 : public Pps {
public:
    Pps4() : Pps(Variant::PPS4) {}
    explicit Pps4(const std::string& s) : Pps(Variant::PPS4) { string_to_it(s); }
    static Pps4 limit(BigInt n);
    static Pps4 master_limit();
    Pps4 operator[](BigInt n) const;   // A[n] = expand(A,n)，不改变 *this
    OrdinalNotation* clone() const override { return new Pps4(*this); }
    std::vector<std::shared_ptr<OrdinalNotation>> roots() const override;
};

class WPps4 : public Pps {
public:
    WPps4() : Pps(Variant::wPPS4) {}
    explicit WPps4(const std::string& s) : Pps(Variant::wPPS4) { string_to_it(s); }
    static WPps4 limit(BigInt n);
    static WPps4 master_limit();
    WPps4 operator[](BigInt n) const;   // A[n] = expand(A,n)，不改变 *this
    OrdinalNotation* clone() const override { return new WPps4(*this); }
    std::vector<std::shared_ptr<OrdinalNotation>> roots() const override;
};

class TPps4 : public Pps {
public:
    TPps4() : Pps(Variant::tPPS4) {}
    explicit TPps4(const std::string& s) : Pps(Variant::tPPS4) { string_to_it(s); }
    static TPps4 limit(BigInt n);
    static TPps4 master_limit();
    TPps4 operator[](BigInt n) const;   // A[n] = expand(A,n)，不改变 *this
    OrdinalNotation* clone() const override { return new TPps4(*this); }
    std::vector<std::shared_ptr<OrdinalNotation>> roots() const override;
};

class FPps4 : public Pps {
public:
    FPps4() : Pps(Variant::fPPS4) {}
    explicit FPps4(const std::string& s) : Pps(Variant::fPPS4) { string_to_it(s); }
    static FPps4 limit(BigInt n);
    static FPps4 master_limit();
    FPps4 operator[](BigInt n) const;   // A[n] = expand(A,n)，不改变 *this
    OrdinalNotation* clone() const override { return new FPps4(*this); }
    std::vector<std::shared_ptr<OrdinalNotation>> roots() const override;
};

} // namespace ordinal
} // namespace googology
