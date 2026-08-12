#include "googology/notations/ordinal/sequence/marked_parent/pps/PPS.hpp"
#include "googology/notations/number/knuth/Knuth.hpp"
#include "googology/core/Notation.hpp"
#include <cassert>
#include <iostream>

using namespace googology::ordinal;

int main() {
    // ---- parse / to_string ----
    assert(Pps1("(0, 1, 0, 3)").to_string() == "(0, 1, 0, 3)");
    assert(Pps1("0,1,0,3").to_string() == "(0, 1, 0, 3)");
    assert(Pps1("[0,1,0,3]").to_string() == "(0, 1, 0, 3)");

    // ---- PPS1 展开：坏根 = 第 x 项（x = 末项值）----
    // (0,1,2): x=2, y=3, 坏根=第2项=1, b=1, L=1。
    //   末项与坏根之间（列标 2..3 开区间）无值=1 → 末项减 1。
    assert(Pps1("(0,1,2)").expand(0).to_string() == "(0, 1, 1)");
    assert(Pps1("(0,1,2)").expand(1).to_string() == "(0, 1, 1, 1)");
    assert(Pps1("(0,1,2)").expand(2).to_string() == "(0, 1, 1, 1, 1)");

    // (0,1,0,3): x=3, y=4, 坏根=第3项=0, b=0, L=1。
    //   末项与坏根之间为空 → 末项减 1 → (0,1,0,2)。
    assert(Pps1("(0,1,0,3)").expand(0).to_string() == "(0, 1, 0, 2)");
    assert(Pps1("(0,1,0,3)").expand(1).to_string() == "(0, 1, 0, 2, 2)");

    // (0,1,0,1,2): x=2, y=5, 坏根=第2项=1, b=1, L=3。
    //   末项与坏根之间存在值=1（第4项）→ 末项换为 b=1；
    //   追加 (0+1)*3-1=2 项（第 i 项: i=3→0, i=4→1）。
    assert(Pps1("(0,1,0,1,2)").expand(0).to_string() == "(0, 1, 0, 1, 1, 0, 1)");

    // 后继：末项为 0 → 一步展开 = 去掉末项。
    assert(Pps1("(0,1,0)").expand(3).to_string() == "(0, 1)");
    assert(Pps1("(0,1,0)").isSuccessor() == true);
    assert(Pps1("(0,1,2)").isSuccessor() == false);

    // expand_to(M) = expandLen(A, M)：替换末项 + 追加 M 项。
    // (0,1,2): expand_to(2) = (0,1,1) + 1,1 = (0,1,1,1,1)。
    assert(Pps1("(0,1,2)").expand_to(2).to_string() == "(0, 1, 1, 1, 1)");

    // 坏根越界（末项值 >= 序列长）→ 资料领域外，防御性抛错。
    {
        bool threw = false;
        try { (void)Pps1("(0,5)").expand(0); }
        catch (const std::out_of_range&) { threw = true; }
        assert(threw);
    }

    // ---- PPS1 vs PPS2：PPS2 需 a_{k+j} > a_{c+j} 才换 b，否则减 1 ----
    // (0,2,4,4,2,3): x=3, y=6, 坏根=第3项=4, b=4, L=3。
    //   PPS1: 存在值=4（第4项）→ 末项换为 4。
    //   PPS2: 比较 a_{4+j} vs a_{3+j}: j=0 平(4=4), j=1: 2<4 → 减 1 → 末项=2。
    assert(Pps1("(0,2,4,4,2,3)").expand(0).to_string() == "(0, 2, 4, 4, 2, 4, 7, 2)");
    assert(Pps2("(0,2,4,4,2,3)").expand(0).to_string() == "(0, 2, 4, 4, 2, 2, 7, 2)");

    // PPS2 无条件减 1 的简单情形（无值=b 的项）与 PPS1 一致：
    assert(Pps2("(0,1,2)").expand(0).to_string() == "(0, 1, 1)");

    // ---- PPS4：弱展开（存在值=b → 换 b）----
    // (0,1,0,1,2): 存在值=1（第4项）→ 弱展开，同 PPS1。
    assert(Pps4("(0,1,0,1,2)").expand(0).to_string() == "(0, 1, 0, 1, 1, 0, 1)");

    // ---- PPS4：强展开，在第 b 列与第 x 列之间找最右侧值=b 的项 ----
    // (0,1,0,2,2,3): x=3, y=6, b=0, L=3。
    //   末项与坏根之间（第4,5项=2,2）无值=0 → 强展开；
    //   第0列与第3列之间（列标 1,2）最右侧值=0 是第1列 → 末项换为 1。
    assert(Pps4("(0,1,0,2,2,3)").expand(0).to_string() == "(0, 1, 0, 2, 2, 1, 2, 2)");
    // PPS1 同例：无值=0 → 末项减 1 → 2。
    assert(Pps1("(0,1,0,2,2,3)").expand(0).to_string() == "(0, 1, 0, 2, 2, 2, 2, 2)");

    // 强展开找不到 → 等同弱展开（换为 b）：
    // (0,1,2,1,3): x=3, y=5, b=2, L=2。第2列与第3列之间为空 → 末项换为 2。
    assert(Pps4("(0,1,2,1,3)").expand(0).to_string() == "(0, 1, 2, 1, 2, 1)");

    // ---- wPPS4：与 PPS4 同（PPS4 正文强展开判定即 =b；注记称 PPS4 原
    // 为 ≤b、wPPS4 收紧为 =b，按字面实现后二者一致）----
    assert(WPps4("(0,1,0,2,2,3)").expand(0).to_string() == "(0, 1, 0, 2, 2, 1, 2, 2)");

    // ---- tPPS4：强展开时由末项复制出的项加阶差 L ----
    // (0,1,0,2,2,3): 强展开后末项=1。expand(1) 追加 2*3-1=5 项，
    //   第 3 个新项源 = 末项（i=6）→ tPPS4 值 = 1+3 = 4（PPS4 为 1）。
    assert(TPps4("(0,1,0,2,2,3)").expand(1).to_string() == "(0, 1, 0, 2, 2, 1, 2, 2, 4, 2, 2)");
    assert(Pps4("(0,1,0,2,2,3)").expand(1).to_string() == "(0, 1, 0, 2, 2, 1, 2, 2, 1, 2, 2)");

    // ---- fPPS4：强展开时末项第一次复制 = 坏根列标 x ----
    assert(FPps4("(0,1,0,2,2,3)").expand(1).to_string() == "(0, 1, 0, 2, 2, 1, 2, 2, 3, 2, 2)");

    // ---- §12 limit 表达式 API ----
    assert(Pps1::limit(0).to_string() == "()");
    assert(Pps1::limit(3).to_string() == "(0, 1, 2)");
    assert(Pps1::master_limit().expand(3).to_string() == "(0, 1, 2)");
    assert(Pps1::master_limit().compare(Pps1::limit(100)) == 1);

    // ---- compare：字典序，仅同变体可比 ----
    assert(Pps1("(0,1,2)").compare(Pps1("(0,1,3)")) == -1);
    assert(Pps1("(0,1,3)").compare(Pps1("(0,1,2)")) == 1);
    assert(Pps1("(0,1,2)").compare(Pps1("(0,1,2)")) == 0);
    assert(Pps1("(0,1,2)").compare(Pps1("(0,1,2,0)")) == -1);
    {
        bool threw = false;
        try { (void)Pps1("(0,1,2)").compare(Pps2("(0,1,2)")); }
        catch (const googology::NotComparable&) { threw = true; }
        assert(threw);
    }
    {
        bool threw = false;
        try { (void)Pps1("(0,1,2)").compare(googology::number::Knuth("2 ^ 3")); }
        catch (const googology::NotComparable&) { threw = true; }
        assert(threw);
    }

    // ---- operator[]（非修改）----
    assert(Pps1("(0,1,2)")[1].to_string() == "(0, 1, 1, 1)");

    // ---- §12 标准型（冒烟）----
    assert(Pps1("(0,1,2)").is_standard() == true);

    // ---- capability report ----
    Pps1 p;
    assert(p.can(googology::Op::FromString));
    assert(p.can(googology::Op::ToString));
    assert(p.can(googology::Op::Normalize));
    assert(p.can(googology::Op::Compare));
    assert(p.can(googology::Op::Expand));
    assert(p.can(googology::Op::ExpandTo));
    assert(p.can(googology::Op::Successor));
    assert(p.family() == googology::Family::Ordinal);
    assert(p.style() == "marked_parent");
    assert(p.creator() == "318`4");
    assert(Pps1().name() == "pps1" && Pps2().name() == "pps2" && Pps4().name() == "pps4"
        && WPps4().name() == "wpps4" && TPps4().name() == "tpps4" && FPps4().name() == "fpps4");

    std::cout << "test_pps: PASS\n";
    return 0;
}
